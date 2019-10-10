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

#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_amount.h"
#include "cfdcore/cfdcore_coin.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_key.h"
#include "cfdcore/cfdcore_logger.h"
#include "cfdcore/cfdcore_script.h"
#include "cfdcore/cfdcore_transaction.h"

namespace cfd {

using cfd::core::AbstractTransaction;
using cfd::core::Address;
using cfd::core::AddressType;
using cfd::core::Amount;
using cfd::core::ByteData;
using cfd::core::CfdError;
using cfd::core::CfdException;
using cfd::core::HashType;
using cfd::core::NetType;
using cfd::core::Pubkey;
using cfd::core::Script;
using cfd::core::ScriptBuilder;
using cfd::core::ScriptOperator;
using cfd::core::ScriptUtil;
using cfd::core::SigHashType;
using cfd::core::SignatureUtil;
using cfd::core::Txid;
using cfd::core::logger::warn;

// -----------------------------------------------------------------------------
// Define
// -----------------------------------------------------------------------------
/// シーケンス値(locktime有効)
constexpr uint32_t kSequenceEnableLockTimeMax = 0xfffffffeU;
/// シーケンス値(locktime無効)
constexpr uint32_t kSequenceDisableLockTime = 0xffffffffU;

// -----------------------------------------------------------------------------
// SignParameter
// -----------------------------------------------------------------------------

SignParameter::SignParameter(
    const ByteData& data, bool der_encode, const SigHashType sighash_type)
    : data_(data),
      data_type_(SignDataType::kSign),
      related_pubkey_(),
      der_encode_(der_encode),
      sighash_type_(sighash_type) {
  // FIXME: encode処理
}

SignParameter::SignParameter(const ByteData& data)
    : data_(data),
      data_type_(SignDataType::kBinary),
      related_pubkey_(),
      der_encode_(false),
      sighash_type_() {
  // do nothing
}

SignParameter::SignParameter(const Pubkey& pubkey)
    : data_(pubkey.GetData()),
      data_type_(SignDataType::kPubkey),
      related_pubkey_(),
      der_encode_(false),
      sighash_type_() {
  // do nothing
}

SignParameter::SignParameter(const Script& redeem_script)
    : data_(redeem_script.GetData()),
      data_type_(SignDataType::kRedeemScript),
      related_pubkey_(),
      der_encode_(false),
      sighash_type_() {
  // do nothing
}

void SignParameter::SetRelatedPubkey(const Pubkey& pubkey) {
  related_pubkey_ = pubkey;
}

ByteData SignParameter::GetData() const { return data_; }

SignDataType SignParameter::GetDataType() const { return data_type_; }

Pubkey SignParameter::GetRelatedPubkey() const { return related_pubkey_; }

bool SignParameter::IsDerEncode() const { return der_encode_; }

SigHashType SignParameter::GetSigHashType() const { return sighash_type_; }

ByteData SignParameter::ConvertToSignature() const {
  // FIXME: signature変換処理
  return ByteData();
}

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
