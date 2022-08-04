#include "yagiaction.hh"
#include "yagiarchitecture.hh"
#include "typemanager.hh"
#include "base.hh"
#include "scope.hh"

namespace yagi 
{
	/**********************************************************************/
	/*!
	 *	\brief	apply data sync with frame space 
	 */
	int4 ActionSyncStackVar::apply(Funcdata& data)
	{
		auto arch = static_cast<YagiArchitecture*>(data.getArch());
		auto funcSym = arch->getSymbolDatabase().find_function(data.getAddress().getOffset());
		auto iter = data.getScopeLocal()->begin();
		while (iter != data.getScopeLocal()->end())
		{
			auto sym = *iter;
			if (sym->getAddr().getSpace()->getName() == "stack")
			{
				auto name = funcSym.value()->findStackVar(
					sym->getAddr().getOffset(), 
					sym->getAddr().getSpace()->getAddrSize()
				);

				if (name.has_value())
				{
					auto high = data.findHigh(sym->getSymbol()->getName());
					if (high != nullptr)
					{
						arch->getLogger().info("Apply stack name override from frame ", high->getSymbol()->getName(), name.value());
						data.getScopeLocal()->renameSymbol(high->getSymbol(), name.value());
					}
					else
					{
						arch->getLogger().info("Apply stack name override from frame ", sym->getSymbol()->getName(), name.value());
						data.getScopeLocal()->renameSymbol(sym->getSymbol(), name.value());
					}
				}
			}
			iter++;
		}
		return 0;
	}

	/**********************************************************************/
	/*!
	 *	\brief	apply data sync with netnode for registry
	 */
	int4 ActionRenameVar::apply(Funcdata& data)
	{
		auto arch = static_cast<YagiArchitecture*>(data.getArch());
		auto funcSym = arch->getSymbolDatabase().find_function(data.getAddress().getOffset());

		auto iter = data.beginOpAll();
		while (iter != data.endOpAll())
		{
			auto op = iter->second;

			uint64_t offset;
			auto newName = funcSym.value()->findName(op->getAddr().getOffset(), m_space, offset);

			auto space = m_space;
			if (space == "const")
			{
				space = "stack";
			}

			auto symEntry = data.getScopeLocal()->findAddr(Address(arch->getSpaceByName(space), offset), op->getAddr());

			if (newName.has_value() && symEntry != nullptr)
			{
				auto sym = symEntry->getSymbol();
				data.getScopeLocal()->renameSymbol(sym,  newName.value());
				data.getScopeLocal()->setAttribute(sym, Varnode::namelock);
			}
			iter++;
		}
		return 0;
	}

	/**********************************************************************/
	int4 ActionLoadLocalScope::apply(Funcdata& data)
	{
		auto arch = static_cast<YagiArchitecture*>(data.getArch());
		auto funcSym = arch->getSymbolDatabase().find_function(data.getAddress().getOffset());

		auto iter = data.beginOpAll();
		while (iter != data.endOpAll())
		{
			auto op = iter->second;

			uint64_t offset;
			auto newType = funcSym.value()->findType(op->getAddr().getOffset(), m_space, offset);

			auto space = m_space;
			if (space == "const")
			{
				space = "stack";
			}
			
			auto opAddr = op->getAddr();

			// for stack based symbol
			// we didn't specify any usepoint
			if (space == "stack")
			{
				opAddr = Address();
			}

			auto symEntry = data.getScopeLocal()->findAddr(Address(arch->getSpaceByName(space), offset), opAddr);

			if (newType.has_value() && data.getScopeLocal()->findAddr(Address(arch->getSpaceByName(space), offset), opAddr) == nullptr)
			{
				if (symEntry == nullptr)
				{
					auto sym = data.getScopeLocal()->addSymbol(
						"",
						static_cast<TypeManager*>(arch->types)->findByTypeInfo(*(newType.value())),
						Address(arch->getSpaceByName(space), offset),
						opAddr
					)->getSymbol();

					data.getScopeLocal()->setAttribute(sym, Varnode::typelock);
					//sym->setIsolated(true);
				}
			}
			iter++;
		}
		return 0;
	}

	/**********************************************************************/
	int4 ActionMIPST9Optimization::apply(Funcdata& data)
	{
		auto arch = static_cast<YagiArchitecture*>(data.getArch());
		auto funcAddr = data.getAddress();
		auto addrSize = data.getArch()->getDefaultCodeSpace()->getAddrSize();

		// We will add a pcode at the begining of the function
		// t9 = funcAddr -> CPUI_COPY input0 = addrFunc; output = t9
		auto beginIter = data.beginOp(funcAddr);
		auto beginPcode = beginIter->second;

		// find t9 register
		auto t9Addr = data.getArch()->getDefaultCodeSpace()->getTrans()->getRegister("t9").getAddr();
		auto t9Vn = data.newVarnode(
			addrSize,
			t9Addr
		);

		auto newPcode = data.newOp(1, funcAddr);
		data.opSetOpcode(newPcode, CPUI_COPY);	// CPUI_COPY
		data.opSetInput(newPcode, data.newConstant(arch->getDefaultCodeSpace()->getAddrSize(), funcAddr.getOffset()), 0); // input = funcAddr
		data.opSetOutput(newPcode, t9Vn);	// output = t9
		data.opInsertBegin(newPcode, beginPcode->getParent());
		data.warningHeader("Yagi : setting t9 register value with address of the current function");
		return 0;
	}

	/**********************************************************************/
	void ActionAddeBPFSyscall::addSyscall(
		YagiArchitecture* arch, 
		const std::string& name, 
		uint32_t syscall_number, 
		Datatype* return_type, 
		Parameters params = Parameters{},
		bool dotdotdot = false
	)
	{
		auto proxy = arch->getYagiScope()->getProxy();
		auto syscall = arch->getSpaceByName("syscall");

		auto sym = proxy->addFunction(Address(syscall, syscall_number), name);
		proxy->addMapPoint(sym, Address(syscall, syscall_number), Address(0, 0));
		auto syscallData = sym->getFunction();
		PrototypePieces pieces;
		syscallData->getFuncProto().getPieces(pieces);

		for (auto param : params)
		{
			pieces.innames.push_back(std::get<0>(param));
			pieces.intypes.push_back(std::get<1>(param));
		}

		pieces.outtype = return_type;
		pieces.dotdotdot = dotdotdot;
		syscallData->getFuncProto().setPieces(pieces);
	}


	/**********************************************************************/
#define INT arch->types->getBase(8, TYPE_INT)
#define PTR arch->types->getTypePointer(8, arch->types->getTypeVoid(), 8)
#define STR arch->types->getTypePointer(8, arch->types->getBase(1, TYPE_INT), 8)
#define U16 arch->types->getBase(2, TYPE_UINT)
#define U32 arch->types->getBase(4, TYPE_UINT)
#define U64 arch->types->getBase(8, TYPE_UINT)
#define S32 arch->types->getBase(4, TYPE_INT)
#define S64 arch->types->getBase(8, TYPE_INT)
#define VOID arch->types->getTypeVoid()

	int4 ActionAddeBPFSyscall::apply(Funcdata& data)
	{
		auto arch = static_cast<YagiArchitecture*>(data.getArch());
		auto proxy = arch->getYagiScope()->getProxy();
		auto syscall = arch->getSpaceByName("syscall");

		
		//void bpf_unspec()
		addSyscall(arch, "bpf_unspec", 0x0, VOID);

		//void *bpf_map_lookup_elem(struct bpf_map *map, const void *key)
		addSyscall(arch, "bpf_map_lookup_elem", 0x1, PTR, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("key", PTR)
			}
		);

		//int bpf_map_update_elem(struct bpf_map *map, const void *key, const void *value, u64 flags)
		addSyscall(arch, "bpf_map_update_elem", 0x3, INT, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("key", PTR),
				std::make_tuple("value", PTR),
				std::make_tuple("flags", U64)
			}
		);

		//int bpf_probe_read(void *dst, u32 size, const void *src)
		addSyscall(arch, "bpf_probe_read", 0x4, INT, Parameters{
				std::make_tuple("dst", PTR),
				std::make_tuple("size", U32),
				std::make_tuple("src", PTR)
			}
		);
		
		//u64 bpf_ktime_get_ns(void)
		addSyscall(arch, "bpf_ktime_get_ns", 0x5, U64);
			
		//int bpf_trace_printk(const char *fmt, u32 fmt_size, ...)
		addSyscall(arch, "bpf_trace_printk", 0x6, INT, Parameters{
				std::make_tuple("fmt", PTR),
				std::make_tuple("fmt_size", U32)
			}, true
		);
			
		//u32 bpf_get_prandom_u32(void)
		addSyscall(arch, "bpf_get_prandom_u32", 0x7, U32);
			
		//u32 bpf_get_smp_processor_id(void)
		addSyscall(arch, "bpf_get_smp_processor_id", 0x8, U32);
			
		//int bpf_skb_store_bytes(struct sk_buff *skb, u32 offset, const void *from, u32 len, u64 flags)
		addSyscall(arch, "bpf_skb_store_bytes", 0x9, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("offset", U32),
				std::make_tuple("from", PTR),
				std::make_tuple("len", U32),
				std::make_tuple("flags", U64)
			}
		);

		//int bpf_l3_csum_replace(struct sk_buff *skb, u32 offset, u64 from, u64 to, u64 size)
		addSyscall(arch, "bpf_l3_csum_replace", 0xa, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("offset", U32),
				std::make_tuple("from", U64),
				std::make_tuple("to", U64),
				std::make_tuple("size", U64)
			}
		);

		//int bpf_l4_csum_replace(struct sk_buff *skb, u32 offset, u64 from, u64 to, u64 flags)
		addSyscall(arch, "bpf_l4_csum_replace", 0xb, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("offset", U32),
				std::make_tuple("from", U64),
				std::make_tuple("to", U64),
				std::make_tuple("flags", U64)
			}
		);

		//int bpf_tail_call(void *ctx, struct bpf_map *prog_array_map, u32 index)
		addSyscall(arch, "bpf_tail_call", 0xc, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("prog_array_map", PTR),
				std::make_tuple("index", U32)
			}
		);

		//int bpf_clone_redirect(struct sk_buff *skb, u32 ifindex, u64 flags)
		addSyscall(arch, "bpf_clone_redirect", 0xd, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("ifindex", U32),
				std::make_tuple("flags", U64)
			}
		);

		//u64 bpf_get_current_pid_tgid(void)
		addSyscall(arch, "bpf_get_current_pid_tgid", 0xe, U64);
			
		//u64 bpf_get_current_uid_gid(void)
		addSyscall(arch, "bpf_get_current_uid_gid", 0xf, U64);

		//int bpf_get_current_comm(char *buf, u32 size_of_buf)
		addSyscall(arch, "bpf_get_current_comm", 0x10, INT, Parameters{
				std::make_tuple("buf", STR),
				std::make_tuple("size_of_buf", U32)
			}
		);

		//u32 bpf_get_cgroup_classid(struct sk_buff *skb)
		addSyscall(arch, "bpf_get_cgroup_classid", 0x11, U32, Parameters{
				std::make_tuple("skb", PTR)
			}
		);
			
		//int bpf_skb_vlan_push(struct sk_buff *skb, __be16 vlan_proto, u16 vlan_tci)
		addSyscall(arch, "bpf_skb_vlan_push", 0x12, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("vlan_proto", U16),
				std::make_tuple("vlan_tci", U16)
			}
		);

		//int bpf_skb_vlan_pop(struct sk_buff *skb)
		addSyscall(arch, "bpf_skb_vlan_pop", 0x13, INT, Parameters{
				std::make_tuple("skb", STR)
			}
		);

		//int bpf_skb_get_tunnel_key(struct sk_buff *skb, struct bpf_tunnel_key *key, u32 size, u64 flags)
		addSyscall(arch, "bpf_skb_get_tunnel_key", 0x14, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("key", PTR),
				std::make_tuple("size", U32),
				std::make_tuple("flags", U64)
			}
		);

		//int bpf_skb_set_tunnel_key(struct sk_buff *skb, struct bpf_tunnel_key *key, u32 size, u64 flags)
		addSyscall(arch, "bpf_skb_set_tunnel_key", 0x15, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("key", PTR),
				std::make_tuple("size", U32),
				std::make_tuple("flags", U64)
			}
		);

		//u64 bpf_perf_event_read(struct bpf_map *map, u64 flags)
		addSyscall(arch, "bpf_perf_event_read", 0x16, U64, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("flags", U64)
			}
		);

		//int bpf_redirect(u32 ifindex, u64 flags)
		addSyscall(arch, "bpf_redirect", 0x17, INT, Parameters{
				std::make_tuple("ifindex", U32),
				std::make_tuple("flags", U64)
			}
		);

		//u32 bpf_get_route_realm(struct sk_buff *skb)
		addSyscall(arch, "bpf_get_route_realm", 0x18, INT, Parameters{
				std::make_tuple("skb", PTR)
			}
		);

		//int bpf_perf_event_output(struct pt_reg *ctx, struct bpf_map *map, u64 flags, void *data, u64 size)
		addSyscall(arch, "bpf_perf_event_output", 0x19, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("map", PTR),
				std::make_tuple("flags", U64),
				std::make_tuple("data", PTR),
				std::make_tuple("size", U64)
			}
		);

		//int bpf_skb_load_bytes(const struct sk_buff *skb, u32 offset, void *to, u32 len)
		addSyscall(arch, "bpf_perf_event_output", 0x1a, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("map", PTR),
				std::make_tuple("flags", U64),
				std::make_tuple("data", PTR),
				std::make_tuple("size", U64)
			}
		);

		//int bpf_get_stackid(struct pt_reg *ctx, struct bpf_map *map, u64 flags)
		addSyscall(arch, "bpf_get_stackid", 0x1b, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("map", PTR),
				std::make_tuple("flags", U64)
			}
		);

		//s64 bpf_csum_diff(__be32 *from, u32 from_size, __be32 *to, u32 to_size, __wsum seed)
		addSyscall(arch, "bpf_csum_diff", 0x1c, S64, Parameters{
				std::make_tuple("from", PTR),
				std::make_tuple("from_size", U32),
				std::make_tuple("to", PTR),
				std::make_tuple("to_size", U32),
				std::make_tuple("seed", U32),
			}
		);

		//int bpf_skb_get_tunnel_opt(struct sk_buff *skb, u8 *opt, u32 size)
		addSyscall(arch, "bpf_skb_get_tunnel_opt", 0x1d, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("opt", PTR),
				std::make_tuple("size", U32)
			}
		);

		//int bpf_skb_set_tunnel_opt(struct sk_buff *skb, u8 *opt, u32 size)
		addSyscall(arch, "bpf_skb_set_tunnel_opt", 0x1e, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("opt", PTR),
				std::make_tuple("size", U32)
			}
		);
			
		//int bpf_skb_change_proto(struct sk_buff *skb, __be16 proto, u64 flags)
		addSyscall(arch, "bpf_skb_change_proto", 0x1f, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("proto", U16),
				std::make_tuple("flags", U64)
			}
		);
			
		//int bpf_skb_change_type(struct sk_buff *skb, u32 type)
		addSyscall(arch, "bpf_skb_change_type", 0x20, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("type", U32)
			}
		);

		//int bpf_skb_under_cgroup(struct sk_buff *skb, struct bpf_map *map, u32 index)
		addSyscall(arch, "bpf_skb_under_cgroup", 0x21, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("map", PTR),
				std::make_tuple("index", U32)
			}
		);
			
		//u32 bpf_get_hash_recalc(struct sk_buff *skb)
		addSyscall(arch, "bpf_get_hash_recalc", 0x22, U32, Parameters{
				std::make_tuple("skb", PTR)
			}
		);

		//u64 bpf_get_current_task(void)
		addSyscall(arch, "bpf_get_current_task", 0x23, U64);

		//int bpf_probe_write_user(void *dst, const void *src, u32 len)
		addSyscall(arch, "bpf_probe_write_user", 0x24, INT, Parameters{
				std::make_tuple("dst", PTR),
				std::make_tuple("src", PTR),
				std::make_tuple("len", U32)
			}
		);

		//int bpf_current_task_under_cgroup(struct bpf_map *map, u32 index)
		addSyscall(arch, "bpf_current_task_under_cgroup", 0x25, INT, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("index", U32)
			}
		);

		//int bpf_skb_change_tail(struct sk_buff *skb, u32 len, u64 flags)
		addSyscall(arch, "bpf_skb_change_tail", 0x26, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("len", U32),
				std::make_tuple("flags", U64)
			}
		);

		//int bpf_skb_pull_data(struct sk_buff *skb, u32 len)
		addSyscall(arch, "bpf_skb_pull_data", 0x27, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("len", U32)
			}
		);
 
		//s64 bpf_csum_update(struct sk_buff *skb, __wsum csum)
		addSyscall(arch, "bpf_csum_update", 0x28, S64, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("csum", U32)
			}
		);

		//void bpf_set_hash_invalid(struct sk_buff *skb)
		addSyscall(arch, "bpf_set_hash_invalid", 0x29, VOID, Parameters{
				std::make_tuple("skb", PTR)
			}
		);

		//int bpf_get_numa_node_id(void)
		addSyscall(arch, "bpf_get_numa_node_id", 0x2a, INT);

		//int bpf_skb_change_head(struct sk_buff *skb, u32 len, u64 flags)
		addSyscall(arch, "bpf_skb_change_head", 0x2b, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("len", U32),
				std::make_tuple("flags", U64)
			}
		);

		//int bpf_xdp_adjust_head(struct xdp_buff *xdp_md, int delta)
		addSyscall(arch, "bpf_xdp_adjust_head", 0x2c, INT, Parameters{
				std::make_tuple("xdp_md", PTR),
				std::make_tuple("delta", INT)
			}
		);

		//int bpf_probe_read_str(void *dst, u32 size, const void *unsafe_ptr)
		addSyscall(arch, "bpf_probe_read_str", 0x2c, INT, Parameters{
				std::make_tuple("dst", PTR),
				std::make_tuple("size", U32),
				std::make_tuple("unsafe_ptr", PTR),
			}
		);

		//u64 bpf_get_socket_cookie(struct sk_buff *skb)
		addSyscall(arch, "bpf_get_socket_cookie", 0x2d, U64, Parameters{
				std::make_tuple("skb", PTR)
			}
		);

		//u64 bpf_get_socket_cookie(struct bpf_sock_addr *ctx)
		addSyscall(arch, "bpf_get_socket_cookie", 0x2e, U64, Parameters{
				std::make_tuple("ctx", PTR)
			}
		);

		//u64 bpf_get_socket_cookie(struct bpf_sock_ops *ctx)
		addSyscall(arch, "bpf_get_socket_cookie", 0x2f, U64, Parameters{
				std::make_tuple("ctx", PTR)
			}
		);

		//u32 bpf_get_socket_uid(struct sk_buff *skb)
		addSyscall(arch, "bpf_get_socket_uid", 0x2f, U32, Parameters{
				std::make_tuple("skb", PTR)
			}
		);

		//int bpf_set_hash(struct sk_buff *skb, u32 hash)
		addSyscall(arch, "bpf_set_hash", 0x30, U32, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("hash", U32)
			}
		);

		//int bpf_setsockopt(void *bpf_socket, int level, int optname, void *optval, int optlen)
		addSyscall(arch, "bpf_setsockopt", 0x31, INT, Parameters{
				std::make_tuple("bpf_socket", PTR),
				std::make_tuple("level", INT),
				std::make_tuple("optname", INT),
				std::make_tuple("optval", PTR),
				std::make_tuple("optlen", INT)
			}
		);
			
		//int bpf_skb_adjust_room(struct sk_buff *skb, s32 len_diff, u32 mode, u64 flags)
		addSyscall(arch, "bpf_skb_adjust_room", 0x32, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("len_diff", S32),
				std::make_tuple("mode", U32),
				std::make_tuple("flags", U64)
			}
		);
			
		//int bpf_redirect_map(struct bpf_map *map, u32 key, u64 flags)
		addSyscall(arch, "bpf_redirect_map", 0x33, INT, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("key", U32),
				std::make_tuple("flags", U64)
			}
		);
			
		//int bpf_sk_redirect_map(struct sk_buff *skb, struct bpf_map *map, u32 key, u64 flags)
		addSyscall(arch, "bpf_sk_redirect_map", 0x34, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("map", PTR),
				std::make_tuple("key", U32),
				std::make_tuple("flags", U64)
			}
		);
			
		//int bpf_sock_map_update(struct bpf_sock_ops *skops, struct bpf_map *map, void *key, u64 flags)
		addSyscall(arch, "bpf_sock_map_update", 0x35, INT, Parameters{
				std::make_tuple("skops", PTR),
				std::make_tuple("map", PTR),
				std::make_tuple("key", PTR),
				std::make_tuple("flags", U64)
			}
		);
			
		//int bpf_xdp_adjust_meta(struct xdp_buff *xdp_md, int delta)
		addSyscall(arch, "bpf_xdp_adjust_meta", 0x36, INT, Parameters{
				std::make_tuple("xdp_md", PTR),
				std::make_tuple("delta", INT)
			}
		);
			
		//int bpf_perf_event_read_value(struct bpf_map *map, u64 flags, struct bpf_perf_event_value *buf, u32 buf_size)
		addSyscall(arch, "bpf_perf_event_read_value", 0x37, INT, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("flags", U64),
				std::make_tuple("buf", PTR),
				std::make_tuple("buf_size", U32)
			}
		);
			
		//int bpf_perf_prog_read_value(struct bpf_perf_event_data *ctx, struct bpf_perf_event_value *buf, u32 buf_size)
		addSyscall(arch, "bpf_perf_prog_read_value", 0x38, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("buf", PTR),
				std::make_tuple("buf_size", U32)
			}
		);

		//int bpf_getsockopt(void *bpf_socket, int level, int optname, void *optval, int optlen)
		addSyscall(arch, "bpf_getsockopt", 0x39, INT, Parameters{
				std::make_tuple("bpf_socket", PTR),
				std::make_tuple("level", INT),
				std::make_tuple("optname", INT),
				std::make_tuple("optval", PTR),
				std::make_tuple("optlen", INT)
			}
		);

		//int bpf_override_return(struct pt_regs *regs, u64 rc)
		addSyscall(arch, "bpf_override_return", 0x3a, INT, Parameters{
				std::make_tuple("regs", PTR),
				std::make_tuple("rc", U64)
			}
		);
			
		//int bpf_sock_ops_cb_flags_set(struct bpf_sock_ops *bpf_sock, int argval)
		addSyscall(arch, "bpf_sock_ops_cb_flags_set", 0x3b, INT, Parameters{
				std::make_tuple("bpf_sock", PTR),
				std::make_tuple("argval", INT)
			}
		);
			
		//int bpf_msg_redirect_map(struct sk_msg_buff *msg, struct bpf_map *map, u32 key, u64 flags)
		addSyscall(arch, "bpf_msg_redirect_map", 0x3c, INT, Parameters{
				std::make_tuple("msg", PTR),
				std::make_tuple("map", PTR),
				std::make_tuple("key", U32),
				std::make_tuple("flags", U64)
			}
		);
			
		//int bpf_msg_apply_bytes(struct sk_msg_buff *msg, u32 bytes)
		addSyscall(arch, "bpf_msg_apply_bytes", 0x3d, INT, Parameters{
				std::make_tuple("msg", PTR),
				std::make_tuple("bytes", U32)
			}
		);

		// long bpf_msg_cork_bytes(struct sk_msg_md *msg, __u32 bytes)
		addSyscall(arch, "bpf_msg_cork_bytes", 0x3e, INT, Parameters{
				std::make_tuple("msg", PTR),
				std::make_tuple("bytes", U32)
			}
		);

		// long bpf_msg_pull_data(struct sk_msg_md *msg, __u32 start, __u32 end, __u64 flags)
		addSyscall(arch, "bpf_msg_pull_data", 0x3f, INT, Parameters{
				std::make_tuple("msg", PTR),
				std::make_tuple("start", U32),
				std::make_tuple("end", U32),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_bind(struct bpf_sock_addr *ctx, struct sockaddr *addr, int addr_len)
		addSyscall(arch, "bpf_bind", 0x40, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("addr", U32),
				std::make_tuple("addr_len", INT)
			}
		);

		// long bpf_xdp_adjust_tail(struct xdp_md *xdp_md, int delta)
		addSyscall(arch, "bpf_xdp_adjust_tail", 0x41, INT, Parameters{
				std::make_tuple("xdp_md", PTR),
				std::make_tuple("delta", INT)
			}
		);

		// long bpf_skb_get_xfrm_state(struct __sk_buff *skb, __u32 index, struct bpf_xfrm_state *xfrm_state, __u32 size, __u64 flags)
		addSyscall(arch, "bpf_skb_get_xfrm_state", 0x42, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("index", INT),
				std::make_tuple("xfrm_state", INT),
				std::make_tuple("size", U32),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_get_stack(void *ctx, void *buf, __u32 size, __u64 flags)
		addSyscall(arch, "bpf_get_stack", 0x43, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("buf", PTR),
				std::make_tuple("size", U32),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_skb_load_bytes_relative(const void *skb, __u32 offset, void *to, __u32 len, __u32 start_header)
		addSyscall(arch, "bpf_skb_load_bytes_relative", 0x44, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("offset", U32),
				std::make_tuple("to", PTR),
				std::make_tuple("len", U32),
				std::make_tuple("start_header", U32)
			}
		);

		// long bpf_fib_lookup(void *ctx, struct bpf_fib_lookup *params, int plen, __u32 flags)
		addSyscall(arch, "bpf_fib_lookup", 0x45, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("params", PTR),
				std::make_tuple("plen", INT),
				std::make_tuple("flags", U32)
			}
		);

		// long bpf_sock_hash_update(struct bpf_sock_ops *skops, void *map, void *key, __u64 flags)
		addSyscall(arch, "bpf_sock_hash_update", 0x46, INT, Parameters{
				std::make_tuple("skops", PTR),
				std::make_tuple("map", PTR),
				std::make_tuple("key", PTR),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_msg_redirect_hash(struct sk_msg_md *msg, void *map, void *key, __u64 flags)
		addSyscall(arch, "bpf_msg_redirect_hash", 0x47, INT, Parameters{
				std::make_tuple("msg", PTR),
				std::make_tuple("map", PTR),
				std::make_tuple("key", PTR),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_sk_redirect_hash(struct __sk_buff *skb, void *map, void *key, __u64 flags)
		addSyscall(arch, "bpf_sk_redirect_hash", 0x48, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("map", PTR),
				std::make_tuple("key", PTR),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_lwt_push_encap(struct __sk_buff *skb, __u32 type, void *hdr, __u32 len)
		addSyscall(arch, "bpf_lwt_push_encap", 0x49, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("type", U32),
				std::make_tuple("hdr", PTR),
				std::make_tuple("len", U32)
			}
		);

		// long bpf_lwt_seg6_store_bytes(struct __sk_buff *skb, __u32 offset, const void *from, __u32 len)
		addSyscall(arch, "bpf_lwt_seg6_store_bytes", 0x4a, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("offset", U32),
				std::make_tuple("from", PTR),
				std::make_tuple("len", U32)
			}
		);

		// long bpf_lwt_seg6_adjust_srh(struct __sk_buff *skb, __u32 offset, __s32 delta)
		addSyscall(arch, "bpf_lwt_seg6_adjust_srh", 0x4b, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("offset", U32),
				std::make_tuple("delta", S32)
			}
		);

		// long bpf_lwt_seg6_action(struct __sk_buff *skb, __u32 action, void *param, __u32 param_len)
		addSyscall(arch, "bpf_lwt_seg6_action", 0x4c, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("action", U32),
				std::make_tuple("param", PTR),
				std::make_tuple("param_len", U32)
			}
		);

		// long bpf_rc_repeat(void *ctx)
		addSyscall(arch, "bpf_rc_repeat", 0x4d, INT, Parameters{
				std::make_tuple("ctx", PTR)
			}
		);

		// long bpf_rc_keydown(void *ctx, __u32 protocol, __u64 scancode, __u32 toggle)
		addSyscall(arch, "bpf_rc_repeat", 0x4e, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("protocol", U32),
				std::make_tuple("scancode", U64),
				std::make_tuple("toggle", U32),
			}
		);
			
		//__u64 bpf_skb_cgroup_id(struct __sk_buff *skb)
		addSyscall(arch, "bpf_skb_cgroup_id", 0x4f, U64, Parameters{
				std::make_tuple("skb", PTR)
			}
		);
			
		// __u64 bpf_get_current_cgroup_id(void)
		addSyscall(arch, "bpf_get_current_cgroup_id", 0x50, U64);

		// void *bpf_get_local_storage(void *map, __u64 flags)
		addSyscall(arch, "bpf_get_local_storage", 0x51, PTR, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("flags", U64)
			}
		);
			
		// long bpf_sk_select_reuseport(struct sk_reuseport_md *reuse, void *map, void *key, __u64 flags)
		addSyscall(arch, "bpf_sk_select_reuseport", 0x52, INT, Parameters{
				std::make_tuple("reuse", PTR),
				std::make_tuple("map", PTR),
				std::make_tuple("key", PTR),
				std::make_tuple("flags", U64)
			}
		);
		
		// __u64 bpf_skb_ancestor_cgroup_id(struct __sk_buff *skb, int ancestor_level)
		addSyscall(arch, "bpf_skb_ancestor_cgroup_id", 0x53, U64, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("ancestor_level", INT)
			}
		);

		// struct bpf_sock *bpf_sk_lookup_tcp(void *ctx, struct bpf_sock_tuple *tuple, __u32 tuple_size, __u64 netns, __u64 flags)
		addSyscall(arch, "bpf_sk_lookup_tcp", 0x54, PTR, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("tuple", PTR),
				std::make_tuple("tuple_size", U32),
				std::make_tuple("netns", U64),
				std::make_tuple("flags", U64),
			}
		);

		// struct bpf_sock *bpf_sk_lookup_udp(void *ctx, struct bpf_sock_tuple *tuple, __u32 tuple_size, __u64 netns, __u64 flags)
		addSyscall(arch, "bpf_sk_lookup_udp", 0x55, PTR, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("tuple", PTR),
				std::make_tuple("tuple_size", U32),
				std::make_tuple("netns", U64),
				std::make_tuple("flags", U64),
			}
		);

		// long bpf_sk_release(void *sock)
		addSyscall(arch, "bpf_sk_release", 0x56, INT, Parameters{
				std::make_tuple("sock", PTR)
			}
		);

		// long bpf_map_push_elem(void *map, const void *value, __u64 flags)
		addSyscall(arch, "bpf_map_push_elem", 0x57, INT, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("value", PTR),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_map_pop_elem(void *map, void *value)
		addSyscall(arch, "bpf_map_pop_elem", 0x58, INT, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("value", PTR)
			}
		);

		// long bpf_map_peek_elem(void *map, void *value)
		addSyscall(arch, "bpf_map_peek_elem", 0x59, INT, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("value", PTR)
			}
		);

		// long bpf_msg_push_data(struct sk_msg_md *msg, __u32 start, __u32 len, __u64 flags)
		addSyscall(arch, "bpf_msg_push_data", 0x5a, INT, Parameters{
				std::make_tuple("msg", PTR),
				std::make_tuple("start", U32),
				std::make_tuple("len", U32),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_msg_pop_data(struct sk_msg_md *msg, __u32 start, __u32 len, __u64 flags)
		addSyscall(arch, "bpf_msg_pop_data", 0x5b, INT, Parameters{
				std::make_tuple("msg", PTR),
				std::make_tuple("start", U32),
				std::make_tuple("len", U32),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_rc_pointer_rel(void *ctx, __s32 rel_x, __s32 rel_y)
		addSyscall(arch, "bpf_rc_pointer_rel", 0x5c, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("rel_x", S32),
				std::make_tuple("rel_y", S32)
			}
		);

		// long bpf_spin_lock(struct bpf_spin_lock *lock)
		addSyscall(arch, "bpf_spin_lock", 0x5d, INT, Parameters{
				std::make_tuple("lock", PTR)
			}
		);

		// long bpf_spin_unlock(struct bpf_spin_lock *lock)
		addSyscall(arch, "bpf_spin_unlock", 0x5e, INT, Parameters{
				std::make_tuple("lock", PTR)
			}
		);

		// struct bpf_sock *bpf_sk_fullsock(struct bpf_sock *sk)
		addSyscall(arch, "bpf_sk_fullsock", 0x5f, PTR, Parameters{
				std::make_tuple("sk", PTR)
			}
		);

		// struct bpf_tcp_sock *bpf_tcp_sock(struct bpf_sock *sk)
		addSyscall(arch, "bpf_tcp_sock", 0x60, PTR, Parameters{
				std::make_tuple("sk", PTR)
			}
		);

		// long bpf_skb_ecn_set_ce(struct __sk_buff *skb)
		addSyscall(arch, "bpf_skb_ecn_set_ce", 0x61, INT, Parameters{
				std::make_tuple("skb", PTR)
			}
		);

		// struct bpf_sock *bpf_get_listener_sock(struct bpf_sock *sk)
		addSyscall(arch, "bpf_get_listener_sock", 0x62, PTR, Parameters{
				std::make_tuple("sk", PTR)
			}
		);

		// struct bpf_sock *bpf_skc_lookup_tcp(void *ctx, struct bpf_sock_tuple *tuple, __u32 tuple_size, __u64 netns, __u64 flags)
		addSyscall(arch, "bpf_skc_lookup_tcp", 0x63, PTR, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("tuple", PTR),
				std::make_tuple("tuple_size", PTR),
				std::make_tuple("netns", U64),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_tcp_check_syncookie(void *sk, void *iph, __u32 iph_len, struct tcphdr *th, __u32 th_len)
		addSyscall(arch, "bpf_tcp_check_syncookie", 0x64, INT, Parameters{
				std::make_tuple("sk", PTR),
				std::make_tuple("iph", PTR),
				std::make_tuple("iph_len", U32),
				std::make_tuple("th", PTR),
				std::make_tuple("th_len", U32)
			}
		);

		// long bpf_sysctl_get_name(struct bpf_sysctl *ctx, char *buf, unsigned long buf_len, __u64 flags)
		addSyscall(arch, "bpf_sysctl_get_name", 0x65, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("buf", STR),
				std::make_tuple("buf_len", U64),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_sysctl_get_current_value(struct bpf_sysctl *ctx, char *buf, unsigned long buf_len)'
		addSyscall(arch, "bpf_sysctl_get_current_value", 0x66, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("buf", STR),
				std::make_tuple("buf_len", U64)
			}
		);

		// long bpf_sysctl_get_new_value(struct bpf_sysctl *ctx, char *buf, unsigned long buf_len)
		addSyscall(arch, "bpf_sysctl_get_new_value", 0x67, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("buf", STR),
				std::make_tuple("buf_len", U64)
			}
		);

		// long bpf_sysctl_set_new_value(struct bpf_sysctl *ctx, const char *buf, unsigned long buf_len)
		addSyscall(arch, "bpf_sysctl_set_new_value", 0x68, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("buf", STR),
				std::make_tuple("buf_len", U64)
			}
		);
		
		// long bpf_strtol(const char *buf, unsigned long buf_len, __u64 flags, long *res)
		addSyscall(arch, "bpf_strtol", 0x69, INT, Parameters{
				std::make_tuple("buf", STR),
				std::make_tuple("buf_len", U64),
				std::make_tuple("flags", U64),
				std::make_tuple("res", PTR)
			}
		);

		// long bpf_strtoul(const char *buf, unsigned long buf_len, __u64 flags, unsigned long *res)
		addSyscall(arch, "bpf_strtoul", 0x6a, INT, Parameters{
				std::make_tuple("buf", STR),
				std::make_tuple("buf_len", U64),
				std::make_tuple("flags", U64),
				std::make_tuple("res", PTR)
			}
		);

		// void *bpf_sk_storage_get(void *map, void *sk, void *value, __u64 flags)
		addSyscall(arch, "bpf_sk_storage_get", 0x6b, PTR, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("sk", PTR),
				std::make_tuple("value", PTR),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_sk_storage_delete(void *map, void *sk)
		addSyscall(arch, "bpf_sk_storage_delete", 0x6c, INT, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("sk", PTR)
			}
		);

		// long bpf_send_signal(__u32 sig)
		addSyscall(arch, "bpf_send_signal", 0x6d, INT, Parameters{
				std::make_tuple("sig", U32)
			}
		);

		// __s64 bpf_tcp_gen_syncookie(void *sk, void *iph, __u32 iph_len, struct tcphdr *th, __u32 th_len)
		addSyscall(arch, "bpf_tcp_gen_syncookie", 0x6e, INT, Parameters{
				std::make_tuple("sk", PTR),
				std::make_tuple("iph", PTR),
				std::make_tuple("iph_len", U32),
				std::make_tuple("th", PTR),
				std::make_tuple("th_len", U32)
			}
		);

		// long bpf_skb_output(void *ctx, void *map, __u64 flags, void *data, __u64 size)
		addSyscall(arch, "bpf_skb_output", 0x6f, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("map", PTR),
				std::make_tuple("flags", U64),
				std::make_tuple("data", PTR),
				std::make_tuple("size", U64)
			}
		);

		// long bpf_probe_read_user(void *dst, __u32 size, const void *unsafe_ptr)
		addSyscall(arch, "bpf_probe_read_user", 0x70, INT, Parameters{
				std::make_tuple("dst", PTR),
				std::make_tuple("size", U32),
				std::make_tuple("unsafe_ptr", PTR)
			}
		);

		// long bpf_probe_read_kernel(void *dst, __u32 size, const void *unsafe_ptr)
		addSyscall(arch, "bpf_probe_read_kernel", 0x71, INT, Parameters{
				std::make_tuple("dst", PTR),
				std::make_tuple("size", U32),
				std::make_tuple("unsafe_ptr", PTR)
			}
		);

		// long bpf_probe_read_user_str(void *dst, __u32 size, const void *unsafe_ptr)
		addSyscall(arch, "bpf_probe_read_user_str", 0x72, INT, Parameters{
				std::make_tuple("dst", STR),
				std::make_tuple("size", U32),
				std::make_tuple("unsafe_ptr", PTR)
			}
		);

		// long bpf_probe_read_kernel_str(void *dst, __u32 size, const void *unsafe_ptr)
		addSyscall(arch, "bpf_probe_read_kernel_str", 0x73, INT, Parameters{
				std::make_tuple("dst", STR),
				std::make_tuple("size", U32),
				std::make_tuple("unsafe_ptr", PTR)
			}
		);

		// long bpf_tcp_send_ack(void *tp, __u32 rcv_nxt)
		addSyscall(arch, "bpf_tcp_send_ack", 0x74, INT, Parameters{
				std::make_tuple("tp", STR),
				std::make_tuple("rcv_nxt", U32)
			}
		);

		// long bpf_send_signal_thread(__u32 sig)
		addSyscall(arch, "bpf_send_signal_thread", 0x75, INT, Parameters{
				std::make_tuple("sig", U32)
			}
		);

		// __u64 bpf_jiffies64(void)
		addSyscall(arch, "bpf_jiffies64", 0x76, U64);

		// long bpf_read_branch_records(struct bpf_perf_event_data *ctx, void *buf, __u32 size, __u64 flags)
		addSyscall(arch, "bpf_read_branch_records", 0x77, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("buf", PTR),
				std::make_tuple("size", U32),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_get_ns_current_pid_tgid(__u64 dev, __u64 ino, struct bpf_pidns_info *nsdata, __u32 size)
		addSyscall(arch, "bpf_get_ns_current_pid_tgid", 0x78, INT, Parameters{
				std::make_tuple("dev", U64),
				std::make_tuple("ino", U64),
				std::make_tuple("nsdata", PTR),
				std::make_tuple("size", U32)
			}
		);

		// long bpf_xdp_output(void *ctx, void *map, __u64 flags, void *data, __u64 size)
		addSyscall(arch, "bpf_xdp_output", 0x79, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("map", PTR),
				std::make_tuple("flags", U64),
				std::make_tuple("data", PTR),
				std::make_tuple("size", U64)
			}
		);

		// __u64 bpf_get_netns_cookie(void *ctx)
		addSyscall(arch, "bpf_get_netns_cookie", 0x7a, INT, Parameters{
				std::make_tuple("ctx", PTR)
			}
		);

		// __u64 bpf_get_current_ancestor_cgroup_id(int ancestor_level)
		addSyscall(arch, "bpf_get_current_ancestor_cgroup_id", 0x7b, U64, Parameters{
				std::make_tuple("ancestor_level", INT)
			}
		);

		// long bpf_sk_assign(void *ctx, void *sk, __u64 flags)
		addSyscall(arch, "bpf_sk_assign", 0x7c, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("sk", PTR),
				std::make_tuple("flags", U64)
			}
		);

		// __u64 bpf_ktime_get_boot_ns(void)
		addSyscall(arch, "bpf_ktime_get_boot_ns", 0x7d, U64);

		// long bpf_seq_printf(struct seq_file *m, const char *fmt, __u32 fmt_size, const void *data, __u32 data_len)
		addSyscall(arch, "bpf_seq_printf", 0x7e, INT, Parameters{
				std::make_tuple("m", PTR),
				std::make_tuple("fmt", STR),
				std::make_tuple("fmt_size", U32),
				std::make_tuple("data", PTR),
				std::make_tuple("data_len", U32)
			}
		);

		// long bpf_seq_write(struct seq_file *m, const void *data, __u32 len)
		addSyscall(arch, "bpf_seq_write", 0x7f, INT, Parameters{
				std::make_tuple("m", PTR),
				std::make_tuple("data", PTR),
				std::make_tuple("len", U32)
			}
		);

		// __u64 bpf_sk_cgroup_id(void *sk)
		addSyscall(arch, "bpf_sk_cgroup_id", 0x80, U64, Parameters{
				std::make_tuple("sk", PTR)
			}
		);

		// __u64 bpf_sk_ancestor_cgroup_id(void *sk, int ancestor_level)
		addSyscall(arch, "bpf_sk_ancestor_cgroup_id", 0x81, U64, Parameters{
				std::make_tuple("sk", PTR),
				std::make_tuple("ancestor_level", INT),
			}
		);

		// long bpf_ringbuf_output(void *ringbuf, void *data, __u64 size, __u64 flags)
		addSyscall(arch, "bpf_ringbuf_output", 0x82, U64, Parameters{
				std::make_tuple("ringbuf", PTR),
				std::make_tuple("data", PTR),
				std::make_tuple("size", U64),
				std::make_tuple("flags", U64)
			}
		);

		// void *bpf_ringbuf_reserve(void *ringbuf, __u64 size, __u64 flags)
		addSyscall(arch, "bpf_ringbuf_reserve", 0x83, PTR, Parameters{
				std::make_tuple("ringbuf", PTR),
				std::make_tuple("data", PTR),
				std::make_tuple("flags", U64)
			}
		);

		// void bpf_ringbuf_submit(void *data, __u64 flags)
		addSyscall(arch, "bpf_ringbuf_submit", 0x84, VOID, Parameters{
				std::make_tuple("ringbuf", PTR),
				std::make_tuple("flags", U64)
			}
		);

		// void bpf_ringbuf_discard(void *data, __u64 flags)
		addSyscall(arch, "bpf_ringbuf_discard", 0x85, VOID, Parameters{
				std::make_tuple("data", PTR),
				std::make_tuple("flags", U64)
			}
		);

		// __u64 bpf_ringbuf_query(void *ringbuf, __u64 flags)
		addSyscall(arch, "bpf_ringbuf_query", 0x86, U64, Parameters{
				std::make_tuple("ringbuf", PTR),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_csum_level(struct __sk_buff *skb, __u64 level)
		addSyscall(arch, "bpf_csum_level", 0x87, INT, Parameters{
				std::make_tuple("skb", PTR),
				std::make_tuple("level", U64)
			}
		);

		// struct tcp6_sock *bpf_skc_to_tcp6_sock(void *sk)
		addSyscall(arch, "bpf_skc_to_tcp6_sock", 0x88, PTR, Parameters{
				std::make_tuple("sk", PTR)
			}
		);

		// struct tcp_sock *bpf_skc_to_tcp_sock(void *sk)
		addSyscall(arch, "bpf_skc_to_tcp_sock", 0x89, PTR, Parameters{
				std::make_tuple("sk", PTR)
			}
		);

		// struct tcp_timewait_sock *bpf_skc_to_tcp_timewait_sock(void *sk)
		addSyscall(arch, "bpf_skc_to_tcp_timewait_sock", 0x8a, PTR, Parameters{
				std::make_tuple("sk", PTR)
			}
		);

		// struct tcp_request_sock *bpf_skc_to_tcp_request_sock(void *sk)
		addSyscall(arch, "bpf_skc_to_tcp_request_sock", 0x8b, PTR, Parameters{
				std::make_tuple("sk", PTR)
			}
		);

		// struct udp6_sock *bpf_skc_to_udp6_sock(void *sk)
		addSyscall(arch, "bpf_skc_to_udp6_sock", 0x8c, PTR, Parameters{
				std::make_tuple("sk", PTR)
			}
		);

		// long bpf_get_task_stack(struct task_struct *task, void *buf, __u32 size, __u64 flags)
		addSyscall(arch, "bpf_get_task_stack", 0x8d, INT, Parameters{
				std::make_tuple("task", PTR),
				std::make_tuple("buf", PTR),
				std::make_tuple("size", U32),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_load_hdr_opt(struct bpf_sock_ops *skops, void *searchby_res, __u32 len, __u64 flags)
		addSyscall(arch, "bpf_load_hdr_opt", 0x8e, INT, Parameters{
				std::make_tuple("skops", PTR),
				std::make_tuple("searchby_res", PTR),
				std::make_tuple("len", U32),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_store_hdr_opt(struct bpf_sock_ops *skops, const void *from, __u32 len, __u64 flags)
		addSyscall(arch, "bpf_store_hdr_opt", 0x8f, INT, Parameters{
				std::make_tuple("skops", PTR),
				std::make_tuple("searchby_res", PTR),
				std::make_tuple("len", U32),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_reserve_hdr_opt(struct bpf_sock_ops *skops, __u32 len, __u64 flags)
		addSyscall(arch, "bpf_reserve_hdr_opt", 0x90, INT, Parameters{
				std::make_tuple("skops", PTR),
				std::make_tuple("len", U32),
				std::make_tuple("flags", U64)
			}
		);

		// void *bpf_inode_storage_get(void *map, void *inode, void *value, __u64 flags)
		addSyscall(arch, "bpf_inode_storage_get", 0x91, PTR, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("inode", PTR),
				std::make_tuple("value", PTR),
				std::make_tuple("flags", U64)
			}
		);

		// int bpf_inode_storage_delete(void *map, void *inode)
		addSyscall(arch, "bpf_inode_storage_delete", 0x92, INT, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("inode", PTR)
			}
		);

		// long bpf_d_path(struct path *path, char *buf, __u32 sz)
		addSyscall(arch, "bpf_d_path", 0x93, INT, Parameters{
				std::make_tuple("path", PTR),
				std::make_tuple("buf", PTR),
				std::make_tuple("sz", U32)
			}
		);

		// long bpf_copy_from_user(void *dst, __u32 size, const void *user_ptr)
		addSyscall(arch, "bpf_copy_from_user", 0x94, INT, Parameters{
				std::make_tuple("dst", PTR),
				std::make_tuple("size", U32),
				std::make_tuple("user_ptr", PTR)
			}
		);

		// long bpf_snprintf_btf(char *str, __u32 str_size, struct btf_ptr *ptr, __u32 btf_ptr_size, __u64 flags)
		addSyscall(arch, "bpf_snprintf_btf", 0x95, INT, Parameters{
				std::make_tuple("str", PTR),
				std::make_tuple("str_size", U32),
				std::make_tuple("ptr", PTR),
				std::make_tuple("ptr_size", U32),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_seq_printf_btf(struct seq_file *m, struct btf_ptr *ptr, __u32 ptr_size, __u64 flags)
		addSyscall(arch, "bpf_seq_printf_btf", 0x96, INT, Parameters{
				std::make_tuple("m", PTR),
				std::make_tuple("ptr", U32),
				std::make_tuple("ptr_size", U32),
				std::make_tuple("flags", U64)
			}
		);

		// __u64 bpf_skb_cgroup_classid(struct __sk_buff *skb)
		addSyscall(arch, "bpf_skb_cgroup_classid", 0x97, INT, Parameters{
				std::make_tuple("skb", PTR)
			}
		);

		// long bpf_redirect_neigh(__u32 ifindex, struct bpf_redir_neigh *params, int plen, __u64 flags)
		addSyscall(arch, "bpf_redirect_neigh", 0x98, INT, Parameters{
				std::make_tuple("ifindex", U32),
				std::make_tuple("params", PTR),
				std::make_tuple("plen", INT),
				std::make_tuple("flags", U64)
			}
		);

		// void *bpf_per_cpu_ptr(const void *percpu_ptr, __u32 cpu)
		addSyscall(arch, "bpf_per_cpu_ptr", 0x99, PTR, Parameters{
				std::make_tuple("percpu_ptr", PTR),
				std::make_tuple("cpu", U32)
			}
		);

		// void *bpf_this_cpu_ptr(const void *percpu_ptr)
		addSyscall(arch, "bpf_this_cpu_ptr", 0x9a, PTR, Parameters{
				std::make_tuple("percpu_ptr", PTR)
			}
		);

		// long bpf_redirect_peer(__u32 ifindex, __u64 flags)
		addSyscall(arch, "bpf_redirect_peer", 0x9b, INT, Parameters{
				std::make_tuple("ifindex", U32),
				std::make_tuple("flags", U64),
			}
		);

		// void *bpf_task_storage_get(void *map, struct task_struct *task, void *value, __u64 flags)
		addSyscall(arch, "bpf_task_storage_get", 0x9c, PTR, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("task", PTR),
				std::make_tuple("value", PTR),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_task_storage_delete(void *map, struct task_struct *task)
		addSyscall(arch, "bpf_task_storage_delete", 0x9d, INT, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("task", PTR)
			}
		);

		// struct task_struct *bpf_get_current_task_btf(void)
		addSyscall(arch, "bpf_get_current_task_btf", 0x9e, PTR);

		// long bpf_bprm_opts_set(struct linux_binprm *bprm, __u64 flags)
		addSyscall(arch, "bpf_bprm_opts_set", 0x9f, INT, Parameters{
				std::make_tuple("bprm", PTR),
				std::make_tuple("flags", U64)
			}
		);

		// __u64 bpf_ktime_get_coarse_ns(void)
		addSyscall(arch, "bpf_ktime_get_coarse_ns", 0xa0, U64);

		// long bpf_ima_inode_hash(struct inode *inode, void *dst, __u32 size)
		addSyscall(arch, "bpf_ima_inode_hash", 0xa1, INT, Parameters{
				std::make_tuple("inode", PTR),
				std::make_tuple("dst", PTR),
				std::make_tuple("size", U32)
			}
		);

		// struct socket *bpf_sock_from_file(struct file *file)
		addSyscall(arch, "bpf_sock_from_file", 0xa2, PTR, Parameters{
				std::make_tuple("file", PTR)
			}
		);

		// long bpf_check_mtu(void *ctx, __u32 ifindex, __u32 *mtu_len, __s32 len_diff, __u64 flags)
		addSyscall(arch, "bpf_check_mtu", 0xa3, INT, Parameters{
				std::make_tuple("ctx", PTR),
				std::make_tuple("ifindex", U32),
				std::make_tuple("mtu_len", PTR),
				std::make_tuple("len_diff", S32),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_for_each_map_elem(void *map, void *callback_fn, void *callback_ctx, __u64 flags)
		addSyscall(arch, "bpf_for_each_map_elem", 0xa4, INT, Parameters{
				std::make_tuple("map", PTR),
				std::make_tuple("callback_fn", PTR),
				std::make_tuple("callback_ctx", PTR),
				std::make_tuple("flags", U64)
			}
		);

		// long bpf_snprintf(char *str, __u32 str_size, const char *fmt, __u64 *data, __u32 data_len)
		addSyscall(arch, "bpf_snprintf", 0xa5, INT, Parameters{
				std::make_tuple("str", STR),
				std::make_tuple("str_size", U32),
				std::make_tuple("fmt", STR),
				std::make_tuple("data", PTR),
				std::make_tuple("data_len", U32)
			}
		);

		return 0;
	}
} // end of namespace yagi