// Copyright 2019 CryptoGarage
/**
 * @file cfd_transaction.cpp
 *
 * @brief Transaction操作共通の関連クラスの実装ファイル
 */
#include "cfd/cfd_transaction_common.h"

#include <algorithm>
#include <string>
#include <vector>

#include "cfd/cfd_script.h"
#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_amount.h"
#include "cfdcore/cfdcore_coin.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_key.h"
#include "cfdcore/cfdcore_logger.h"
#include "cfdcore/cfdcore_script.h"
#include "cfdcore/cfdcore_transaction.h"

namespace cfd {

using cfdcore::AbstractTransaction;
using cfdcore::Address;
using cfdcore::AddressType;
using cfdcore::Amount;
using cfdcore::ByteData;
using cfdcore::CfdError;
using cfdcore::CfdException;
using cfdcore::HashType;
using cfdcore::NetType;
using cfdcore::Pubkey;
using cfdcore::Script;
using cfdcore::ScriptBuilder;
using cfdcore::ScriptOperator;
using cfdcore::SigHashType;
using cfdcore::SignatureUtil;
using cfdcore::Txid;
using cfdcore::logger::warn;

using cfd::ScriptUtil;

// -----------------------------------------------------------------------------
// Define
// -----------------------------------------------------------------------------
/// シーケンス値(locktime有効)
constexpr uint32_t kSequenceEnableLockTimeMax = 0xfffffffeU;
/// シーケンス値(locktime無効)
constexpr uint32_t kSequenceDisableLockTime = 0xffffffffU;

// -----------------------------------------------------------------------------
// TransactionController
// -----------------------------------------------------------------------------
AbstractTransactionController::AbstractTransactionController()
    : tx_address_(nullptr) {
  // do nothing
}

std::string AbstractTransactionController::GetHex() const {
  return tx_address_->GetHex();
}

uint32_t AbstractTransactionController::GetLockTimeDisabledSequence() {
  return kSequenceDisableLockTime;
}

uint32_t AbstractTransactionController::GetDefaultSequence() {
  if (tx_address_->GetLockTime() == 0) {
    return kSequenceDisableLockTime;
  } else {
    return kSequenceEnableLockTimeMax;
  }
}
}  // namespace cfd
