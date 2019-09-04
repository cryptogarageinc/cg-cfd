// Copyright 2019 CryptoGarage
/**
 * @file cfd_script.cpp
 *
 * @brief Script生成Utilクラス定義
 */
#include <vector>

#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_logger.h"
#include "cfdcore/cfdcore_script.h"
#include "cfdcore/cfdcore_util.h"

#include "cfd/cfd_script.h"

namespace cfd {

using cfdcore::ByteData160;
using cfdcore::ByteData256;
using cfdcore::CfdError;
using cfdcore::CfdException;
using cfdcore::HashUtil;
using cfdcore::Pubkey;
using cfdcore::Script;
using cfdcore::ScriptBuilder;
using cfdcore::ScriptElement;
using cfdcore::ScriptOperator;
using cfdcore::logger::warn;

// <pubkey> OP_CHECKSIG
Script ScriptUtil::CreateP2pkLockingScript(const Pubkey& pubkey) {
  // script作成
  ScriptBuilder builder;
  builder.AppendData(pubkey);
  builder.AppendOperator(ScriptOperator::OP_CHECKSIG);

  return builder.Build();
}

// OP_DUP OP_HASH160 <hash160(pubkey)> OP_EQUALVERIFY OP_CHECKSIG
Script ScriptUtil::CreateP2pkhLockingScript(const ByteData160& pubkey_hash) {
  // script作成
  ScriptBuilder builder;
  builder.AppendOperator(ScriptOperator::OP_DUP);
  builder.AppendOperator(ScriptOperator::OP_HASH160);
  builder.AppendData(pubkey_hash);
  builder.AppendOperator(ScriptOperator::OP_EQUALVERIFY);
  builder.AppendOperator(ScriptOperator::OP_CHECKSIG);

  return builder.Build();
}

// OP_DUP OP_HASH160 <hash160(pubkey)> OP_EQUALVERIFY OP_CHECKSIG
Script ScriptUtil::CreateP2pkhLockingScript(const Pubkey& pubkey) {
  // pubkey hash作成
  ByteData160 pubkey_hash = HashUtil::Hash160(pubkey);

  return CreateP2pkhLockingScript(pubkey_hash);
}

// OP_HASH160 <hash160(redeem_script)> OP_EQUAL
Script ScriptUtil::CreateP2shLockingScript(const ByteData160& script_hash) {
  // script作成
  ScriptBuilder builder;
  builder.AppendOperator(ScriptOperator::OP_HASH160);
  builder.AppendData(script_hash);
  builder.AppendOperator(ScriptOperator::OP_EQUAL);

  return builder.Build();
}

// OP_HASH160 <hash160(redeem_script)> OP_EQUAL
Script ScriptUtil::CreateP2shLockingScript(const Script& redeem_script) {
  // script hash作成
  ByteData160 script_hash = HashUtil::Hash160(redeem_script);

  return CreateP2shLockingScript(script_hash);
}

// OP_0 <hash160(pubkey)>
Script ScriptUtil::CreateP2wpkhLockingScript(const ByteData160& pubkey_hash) {
  // script作成
  ScriptBuilder builder;
  builder.AppendOperator(ScriptOperator::OP_0);
  builder.AppendData(pubkey_hash);

  return builder.Build();
}

// OP_0 <hash160(pubkey)>
Script ScriptUtil::CreateP2wpkhLockingScript(const Pubkey& pubkey) {
  // pubkey hash作成
  ByteData160 pubkey_hash = HashUtil::Hash160(pubkey);

  return CreateP2wpkhLockingScript(pubkey_hash);
}

// OP_0 <sha256(redeem_script)>
Script ScriptUtil::CreateP2wshLockingScript(const ByteData256& script_hash) {
  // script作成
  ScriptBuilder builder;
  builder.AppendOperator(ScriptOperator::OP_0);
  builder.AppendData(script_hash);

  return builder.Build();
}

// OP_0 <sha256(redeem_script)>
Script ScriptUtil::CreateP2wshLockingScript(const Script& redeem_script) {
  // script hash作成
  ByteData256 script_hash = HashUtil::Sha256(redeem_script);

  return CreateP2wshLockingScript(script_hash);
}

bool ScriptUtil::IsValidRedeemScript(const Script& redeem_script) {
  size_t script_buf_size = redeem_script.GetData().GetDataSize();
  if (script_buf_size > Script::kMaxRedeemScriptSize) {
    warn(
        CFD_LOG_SOURCE, "Redeem script size is over the limit. script size={}",
        script_buf_size);
    return false;
  }
  return true;
}

// OP_n <pubkey> ... OP_<requireSigNum> OP_CHECKMULTISIG
Script ScriptUtil::CreateMultisigRedeemScript(
    uint32_t require_signature_num, const std::vector<Pubkey>& pubkeys) {
  if (require_signature_num == 0) {
    warn(CFD_LOG_SOURCE, "Invalid require_sig_num. require_sig_num = 0");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "CreateMultisigScript require_num is 0.");
  }
  if (pubkeys.empty()) {
    warn(CFD_LOG_SOURCE, "pubkey array is empty.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "CreateMultisigScript empty pubkey array.");
  }
  if (require_signature_num > pubkeys.size()) {
    warn(
        CFD_LOG_SOURCE,
        "Invalid require_sig_num. require_sig_num={0}, pubkey size={1}.",
        require_signature_num, pubkeys.size());
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "CreateMultisigScript require_num is over.");
  }
  if (pubkeys.size() > 15) {
    warn(CFD_LOG_SOURCE, "pubkey array size is over.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "CreateMultisigScript pubkeys array size is over.");
  }

  ScriptElement op_require_num(static_cast<int64_t>(require_signature_num));
  ScriptElement op_pubkey_num(static_cast<int64_t>(pubkeys.size()));

  // script作成
  ScriptBuilder builder;
  builder.AppendOperator(op_require_num.GetOpCode());
  for (const Pubkey& pubkey : pubkeys) {
    builder.AppendData(pubkey);
  }
  builder.AppendOperator(op_pubkey_num.GetOpCode());
  builder.AppendOperator(ScriptOperator::OP_CHECKMULTISIG);
  Script redeem_script = builder.Build();

  if (!IsValidRedeemScript(redeem_script)) {
    warn(CFD_LOG_SOURCE, "Multisig script size is over.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "CreateMultisigScript multisig script size is over.");
  }
  return redeem_script;
}

}  // namespace cfd
