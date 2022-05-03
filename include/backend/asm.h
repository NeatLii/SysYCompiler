#ifndef __sysycompiler_backend_asm_h__
#define __sysycompiler_backend_asm_h__

#include <memory>

#include "backend/instruction.h"
#include "ir/ir.h"

namespace backend {

class RegPool {
  public:
    RegPool() noexcept = default;

    std::shared_ptr<RegOperand> Get(int id);
    std::shared_ptr<RegOperand> operator[](int id);

  private:
    std::vector<std::shared_ptr<RegOperand>> reg_pool;
};

void TranslateGlobalVar(const std::shared_ptr<ir::GlobalVarDef> &var_def);
void TranslateFunction(const std::shared_ptr<ir::FuncDef> &func_def);

void TranslateBasicBlock(const std::shared_ptr<Function> &func,
                         const std::shared_ptr<ir::BasicBlock> &bb);

void TranslateInst(const std::shared_ptr<Function> &func, ir::Inst &inst);
void TranslateRetInst(const std::shared_ptr<Function> &func,
                      const ir::RetInst &inst);
void TranslateBrInst(const std::shared_ptr<Function> &func,
                     const ir::BrInst &inst);
void TranslateBinaryOpInst(const std::shared_ptr<Function> &func,
                           const ir::BinaryOpInst &inst);
void TranslateAllocaInst(const std::shared_ptr<Function> &func,
                         const ir::AllocaInst &inst);
void TranslateLoadInst(const std::shared_ptr<Function> &func,
                       const ir::LoadInst &inst);
void TranslateStoreInst(const std::shared_ptr<Function> &func,
                        const ir::StoreInst &inst);
void TranslateGetelementptrInst(const std::shared_ptr<Function> &func,
                                const ir::GetelementptrInst &inst);
void TranslateBitcastInst(const std::shared_ptr<Function> &func,
                          const ir::BitcastInst &inst);

}  // namespace backend

#endif
