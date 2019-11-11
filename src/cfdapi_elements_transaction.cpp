// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_elements_transaction.cpp
 *
 * @brief cfd-apiで利用するConfidential Transaction操作の実装ファイル
 */
#ifndef CFD_DISABLE_ELEMENTS
#include <algorithm>
#include <limits>
#include <set>
#include <string>
#include <vector>

#include "cfd/cfd_elements_transaction.h"
#include "cfd/cfd_fee.h"
#include "cfd_manager.h"  // NOLINT
#include "cfdcore/cfdcore_amount.h"
#include "cfdcore/cfdcore_bytedata.h"
#include "cfdcore/cfdcore_coin.h"
#include "cfdcore/cfdcore_elements_address.h"
#include "cfdcore/cfdcore_elements_transaction.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_key.h"
#include "cfdcore/cfdcore_logger.h"
#include "cfdcore/cfdcore_script.h"
#include "cfdcore/cfdcore_transaction.h"
#include "cfdcore/cfdcore_util.h"

#include "cfd/cfd_address.h"
#include "cfd/cfd_elements_address.h"
#include "cfd/cfdapi_address.h"
#include "cfd/cfdapi_elements_address.h"
#include "cfd/cfdapi_elements_transaction.h"
#include "cfd/cfdapi_transaction.h"
#include "cfdapi_transaction_base.h"  // NOLINT

namespace cfd {
namespace api {

using cfd::ConfidentialTransactionController;
using cfd::FeeCalculator;
using cfd::SignParameter;
using cfd::api::TransactionApiBase;
using cfd::core::Address;
using cfd::core::AddressType;
using cfd::core::Amount;
using cfd::core::BlindParameter;
using cfd::core::ByteData;
using cfd::core::ByteData160;
using cfd::core::CfdError;
using cfd::core::CfdException;
using cfd::core::ConfidentialTransaction;
using cfd::core::ConfidentialTxIn;
using cfd::core::ConfidentialTxInReference;
using cfd::core::ConfidentialTxOut;
using cfd::core::ConfidentialTxOutReference;
using cfd::core::ConfidentialValue;
using cfd::core::ExtPubkey;
using cfd::core::HashType;
using cfd::core::HashUtil;
using cfd::core::IssuanceBlindingKeyPair;
using cfd::core::IssuanceParameter;
using cfd::core::Privkey;
using cfd::core::Pubkey;
using cfd::core::Script;
using cfd::core::ScriptUtil;
using cfd::core::SigHashType;
using cfd::core::Txid;
using cfd::core::UnblindParameter;
using cfd::core::WitnessVersion;
using cfd::core::logger::info;
using cfd::core::logger::warn;

// -----------------------------------------------------------------------------
// ファイル内関数
// -----------------------------------------------------------------------------

/**
 * @brief Create a ConfidentialTransactionController object.
 *
 * @param[in] hex of the transaction for which to create the controller object
 * @return a ConfidentialTransactionController instance
 */
static ConfidentialTransactionController CreateController(
    const std::string& hex) {
  return ConfidentialTransactionController(hex);
}

ConfidentialTransactionController ElementsTransactionApi::CreateRawTransaction(
    uint32_t version, uint32_t locktime,
    const std::vector<ConfidentialTxIn>& txins,
    const std::vector<ConfidentialTxOut>& txouts,
    const ConfidentialTxOut& txout_fee) const {
  // Transaction作成
  ConfidentialTransactionController ctxc(version, locktime);

  // TxInの追加
  const uint32_t kLockTimeDisabledSequence =
      ctxc.GetLockTimeDisabledSequence();
  for (const auto& txin : txins) {
    // TxInのunlocking_scriptは空で作成
    if (kLockTimeDisabledSequence == txin.GetSequence()) {
      ctxc.AddTxIn(txin.GetTxid(), txin.GetVout(), ctxc.GetDefaultSequence());
    } else {
      ctxc.AddTxIn(txin.GetTxid(), txin.GetVout(), txin.GetSequence());
    }
  }

  // TxOutの追加
  for (const auto& txout : txouts) {
    ctxc.AddTxOut(
        txout.GetLockingScript(), txout.GetConfidentialValue().GetAmount(),
        txout.GetAsset(), txout.GetNonce());
  }

  // amountが0のfeeは無効と判定
  if (txout_fee.GetConfidentialValue().GetAmount() != 0) {
    ctxc.AddTxOutFee(
        txout_fee.GetConfidentialValue().GetAmount(), txout_fee.GetAsset());
  }

  return ctxc;
}

uint32_t ElementsTransactionApi::GetWitnessStackNum(
    const std::string& tx_hex, const Txid& txid, uint32_t vout) const {
  return TransactionApiBase::GetWitnessStackNum<
      ConfidentialTransactionController>(
      cfd::api::CreateController, tx_hex, txid, vout);
}

ConfidentialTransactionController ElementsTransactionApi::AddSign(
    const std::string& hex, const Txid& txid, uint32_t vout,
    const std::vector<SignParameter>& sign_params, bool is_witness,
    bool clear_stack) const {
  return TransactionApiBase::AddSign<ConfidentialTransactionController>(
      cfd::api::CreateController, hex, txid, vout, sign_params, is_witness,
      clear_stack);
}

ConfidentialTransactionController ElementsTransactionApi::UpdateWitnessStack(
    const std::string& tx_hex, const Txid& txid, uint32_t vout,
    const SignParameter& update_sign_param, uint32_t stack_index) const {
  return TransactionApiBase::UpdateWitnessStack<
      ConfidentialTransactionController>(
      cfd::api::CreateController, tx_hex, txid, vout, update_sign_param,
      stack_index);
}

ByteData ElementsTransactionApi::CreateSignatureHash(
    const std::string& tx_hex, const ConfidentialTxInReference& txin,
    const Pubkey& pubkey, const ConfidentialValue& value, HashType hash_type,
    const SigHashType& sighash_type) const {
  return CreateSignatureHash(
      tx_hex, txin, pubkey.GetData(), value, hash_type, sighash_type);
}

ByteData ElementsTransactionApi::CreateSignatureHash(
    const std::string& tx_hex, const ConfidentialTxInReference& txin,
    const Script& redeem_script, const ConfidentialValue& value,
    HashType hash_type, const SigHashType& sighash_type) const {
  return CreateSignatureHash(
      tx_hex, txin, redeem_script.GetData(), value, hash_type, sighash_type);
}

ByteData ElementsTransactionApi::CreateSignatureHash(
    const std::string& tx_hex, const ConfidentialTxInReference& txin,
    const ByteData& key_data, const ConfidentialValue& value,
    HashType hash_type, const SigHashType& sighash_type) const {
  return CreateSignatureHash(
      tx_hex, txin.GetTxid(), txin.GetVout(), key_data, value, hash_type,
      sighash_type);
}

ByteData ElementsTransactionApi::CreateSignatureHash(
    const std::string& tx_hex, const Txid& txid, uint32_t vout,
    const ByteData& key_data, const ConfidentialValue& value,
    HashType hash_type, const SigHashType& sighash_type) const {
  std::string sig_hash;
  ConfidentialTransactionController txc(tx_hex);
  bool is_witness = false;

  switch (hash_type) {
    case HashType::kP2pkh:
      // fall-through
    case HashType::kP2wpkh:
      if (hash_type == HashType::kP2wpkh) {
        is_witness = true;
      }
      if (value.HasBlinding()) {
        sig_hash = txc.CreateSignatureHash(
            txid, vout, Pubkey(key_data), sighash_type, value.GetData(),
            is_witness);
      } else {
        sig_hash = txc.CreateSignatureHash(
            txid, vout, Pubkey(key_data), sighash_type, value.GetAmount(),
            is_witness);
      }
      break;
    case HashType::kP2sh:
      // fall-through
    case HashType::kP2wsh:
      if (hash_type == HashType::kP2wsh) {
        is_witness = true;
      }
      if (value.HasBlinding()) {
        sig_hash = txc.CreateSignatureHash(
            txid, vout, Script(key_data), sighash_type, value.GetData(),
            is_witness);
      } else {
        sig_hash = txc.CreateSignatureHash(
            txid, vout, Script(key_data), sighash_type, value.GetAmount(),
            is_witness);
      }
      break;
    default:
      warn(
          CFD_LOG_SOURCE,
          "Failed to CreateSignatureHash. Invalid hash_type: {}", hash_type);
      throw CfdException(
          CfdError::kCfdIllegalArgumentError, "Invalid hash_type.");
      break;
  }

  return ByteData(sig_hash);
}

ConfidentialTransactionController ElementsTransactionApi::AddMultisigSign(
    const std::string& tx_hex, const ConfidentialTxInReference& txin,
    const std::vector<SignParameter>& sign_list, AddressType address_type,
    const Script& witness_script, const Script redeem_script,
    bool clear_stack) {
  return AddMultisigSign(
      tx_hex, txin.GetTxid(), txin.GetVout(), sign_list, address_type,
      witness_script, redeem_script, clear_stack);
}

ConfidentialTransactionController ElementsTransactionApi::AddMultisigSign(
    const std::string& tx_hex, const Txid& txid, uint32_t vout,
    const std::vector<SignParameter>& sign_list, AddressType address_type,
    const Script& witness_script, const Script redeem_script,
    bool clear_stack) {
  std::string result =
      TransactionApiBase::AddMultisigSign<ConfidentialTransactionController>(
          CreateController, tx_hex, txid, vout, sign_list, address_type,
          witness_script, redeem_script, clear_stack);
  return ConfidentialTransactionController(result);
}

ConfidentialTransactionController ElementsTransactionApi::BlindTransaction(
    const std::string& tx_hex,
    const std::vector<TxInBlindParameters>& txin_blind_keys,
    const std::vector<TxOutBlindKeys>& txout_blind_keys,
    bool is_issuance_blinding) {
  ConfidentialTransactionController txc(tx_hex);

  uint32_t txin_count = txc.GetTransaction().GetTxInCount();
  uint32_t txout_count = txc.GetTransaction().GetTxOutCount();

  if (txin_blind_keys.size() == 0) {
    warn(CFD_LOG_SOURCE, "Failed to txins empty.");
    throw CfdException(
        CfdError::kCfdOutOfRangeError, "JSON value error. Empty txins.");
  }
  if (txout_blind_keys.size() == 0) {
    warn(CFD_LOG_SOURCE, "Failed to txouts empty.");
    throw CfdException(
        CfdError::kCfdOutOfRangeError, "JSON value error. Empty txouts.");
  }

  std::vector<BlindParameter> txin_info_list(txin_count);
  std::vector<Pubkey> txout_confidential_keys(txout_count);
  std::vector<IssuanceBlindingKeyPair> issuance_blinding_keys;
  if (is_issuance_blinding) {
    issuance_blinding_keys.resize(txin_count);
  }

  // TxInのBlind情報設定
  for (TxInBlindParameters txin_key : txin_blind_keys) {
    uint32_t index =
        txc.GetTransaction().GetTxInIndex(txin_key.txid, txin_key.vout);
    txin_info_list[index].asset = txin_key.blind_param.asset;
    txin_info_list[index].vbf = txin_key.blind_param.vbf;
    txin_info_list[index].abf = txin_key.blind_param.abf;
    txin_info_list[index].value = txin_key.blind_param.value;
    if (txin_key.is_issuance) {
      issuance_blinding_keys[index].asset_key =
          txin_key.issuance_key.asset_key;
      issuance_blinding_keys[index].token_key =
          txin_key.issuance_key.token_key;
    }
  }

  // TxOutのBlind情報設定
  for (TxOutBlindKeys txout_key : txout_blind_keys) {
    if (txout_key.index < txout_count) {
      txout_confidential_keys[txout_key.index] = txout_key.blinding_key;
    } else {
      warn(
          CFD_LOG_SOURCE,
          "Failed to BlindTransaction. Invalid txout index: {}",
          txout_key.index);
      throw CfdException(
          CfdError::kCfdIllegalArgumentError, "Invalid txout index.");
    }
  }

  txc.BlindTransaction(
      txin_info_list, issuance_blinding_keys, txout_confidential_keys);
  return txc;
}

ConfidentialTransactionController ElementsTransactionApi::UnblindTransaction(
    const std::string& tx_hex,
    const std::vector<TxOutUnblindKeys>& txout_unblind_keys,
    const std::vector<IssuanceBlindKeys>& issuance_blind_keys,
    std::vector<UnblindOutputs>* blind_outputs,
    std::vector<UnblindIssuanceOutputs>* issuance_outputs) {
  ConfidentialTransactionController ctxc(tx_hex);

  if (!txout_unblind_keys.empty() && blind_outputs != nullptr) {
    UnblindParameter unblind_param;
    for (const auto& txout : txout_unblind_keys) {
      // TxOutをUnblind
      const Privkey blinding_key(txout.blinding_key);
      unblind_param = ctxc.UnblindTxOut(txout.index, blinding_key);

      if (!unblind_param.asset.GetHex().empty()) {
        UnblindOutputs output;
        output.index = txout.index;
        output.blind_param.asset = unblind_param.asset;
        output.blind_param.vbf = unblind_param.vbf;
        output.blind_param.abf = unblind_param.abf;
        output.blind_param.value = unblind_param.value;
        blind_outputs->push_back(output);
      }
    }
  }

  if (!issuance_blind_keys.empty() && issuance_outputs != nullptr) {
    for (const auto& issuance : issuance_blind_keys) {
      uint32_t txin_index = ctxc.GetTransaction().GetTxInIndex(
          Txid(issuance.txid), issuance.vout);

      std::vector<UnblindParameter> issuance_param = ctxc.UnblindIssuance(
          txin_index, issuance.issuance_key.asset_key,
          issuance.issuance_key.token_key);

      UnblindIssuanceOutputs output;
      output.txid = issuance.txid;
      output.vout = issuance.vout;
      output.asset = issuance_param[0].asset;
      output.asset_amount = issuance_param[0].value;
      if (issuance_param.size() > 1) {
        output.token = issuance_param[1].asset;
        output.token_amount = issuance_param[1].value;
      }
      issuance_outputs->push_back(output);
    }
  }
  return ctxc;
}

ConfidentialTransactionController ElementsTransactionApi::SetRawIssueAsset(
    const std::string& tx_hex,
    const std::vector<TxInIssuanceParameters>& issuances,
    std::vector<IssuanceOutput>* issuance_output) {
  ConfidentialTransactionController ctxc(tx_hex);

  for (const auto& issuance : issuances) {
    Script asset_locking_script = issuance.asset_txout.GetLockingScript();
    ByteData asset_nonce = issuance.asset_txout.GetNonce().GetData();
    Script token_locking_script = issuance.token_txout.GetLockingScript();
    ByteData token_nonce = issuance.token_txout.GetNonce().GetData();

    IssuanceParameter issuance_param = ctxc.SetAssetIssuance(
        issuance.txid, issuance.vout, issuance.asset_amount,
        asset_locking_script, asset_nonce, issuance.token_amount,
        token_locking_script, token_nonce, issuance.is_blind,
        issuance.contract_hash, false);

    if (issuance_output != nullptr) {
      IssuanceOutput output;
      output.txid = issuance.txid;
      output.vout = issuance.vout;
      output.output.asset = issuance_param.asset;
      output.output.entropy = issuance_param.entropy;
      output.output.token = issuance_param.token;
      issuance_output->push_back(output);
    }
  }
  return ctxc;
}

ConfidentialTransactionController ElementsTransactionApi::SetRawReissueAsset(
    const std::string& tx_hex,
    const std::vector<TxInReissuanceParameters>& issuances,
    std::vector<IssuanceOutput>* issuance_output) {
  ConfidentialTransactionController ctxc(tx_hex);

  for (const auto& issuance : issuances) {
    Script locking_script = issuance.asset_txout.GetLockingScript();
    ByteData nonce = issuance.asset_txout.GetNonce().GetData();

    IssuanceParameter issuance_param = ctxc.SetAssetReissuance(
        issuance.txid, issuance.vout, issuance.amount, locking_script, nonce,
        issuance.blind_factor, issuance.entropy, false);

    if (issuance_output != nullptr) {
      IssuanceOutput output;
      output.txid = issuance.txid;
      output.vout = issuance.vout;
      output.output.asset = issuance_param.asset;
      output.output.entropy = issuance_param.entropy;
      issuance_output->push_back(output);
    }
  }
  return ctxc;
}

ConfidentialTransactionController
ElementsTransactionApi::CreateRawPeginTransaction(
    uint32_t version, uint32_t locktime,
    const std::vector<ConfidentialTxIn>& txins,
    const std::vector<TxInPeginParameters>& pegins,
    const std::vector<ConfidentialTxOut>& txouts,
    const ConfidentialTxOut& txout_fee) const {
  ConfidentialTransactionController ctxc =
      CreateRawTransaction(version, locktime, txins, txouts, txout_fee);

  for (const auto& pegin_data : pegins) {
    ctxc.AddPeginWitness(
        pegin_data.txid, pegin_data.vout, pegin_data.amount, pegin_data.asset,
        pegin_data.mainchain_blockhash, pegin_data.claim_script,
        pegin_data.mainchain_raw_tx, pegin_data.mainchain_txoutproof);
  }

  return ctxc;
}

ConfidentialTransactionController
ElementsTransactionApi::CreateRawPegoutTransaction(
    uint32_t version, uint32_t locktime,
    const std::vector<ConfidentialTxIn>& txins,
    const std::vector<ConfidentialTxOut>& txouts,
    const TxOutPegoutParameters& pegout_data,
    const ConfidentialTxOut& txout_fee, Address* pegout_address) const {
  ConfidentialTxOut empty_fee;
  ConfidentialTransactionController ctxc =
      CreateRawTransaction(version, locktime, txins, txouts, empty_fee);

  // PegoutのTxOut追加
  const std::string pegout_addr_string = pegout_data.btc_address.GetAddress();

  if (pegout_data.online_pubkey.IsValid() &&
      !pegout_data.master_online_key.IsInvalid()) {
    Address pegout_addr;
    if (pegout_addr_string.empty()) {
      // TODO(k-matsuzawa): ExtKeyの正式対応が入るまでの暫定対応
      // pegoutのtemplateに従い、xpub/counterから生成する
      // descriptor parse
      std::string desc = pegout_data.bitcoin_descriptor;
      std::string::size_type start_point = desc.rfind('(');
      std::string arg_type;
      std::string xpub;
      if (start_point == std::string::npos) {
        xpub = desc;
      } else {
        arg_type = desc.substr(0, start_point);
        xpub = desc.substr(start_point + 1);
      }
      std::string::size_type end_point = xpub.find('/');
      if (end_point == std::string::npos) {
        end_point = xpub.find(')');
        if (end_point != std::string::npos) {
          xpub = xpub.substr(0, end_point);
        }
      } else {
        xpub = xpub.substr(0, end_point);
      }
      // info(CFD_LOG_SOURCE, "arg_type={}, xpub={}", arg_type, xpub);
      // key生成
      std::vector<uint32_t> path = {0, pegout_data.bip32_counter};
      ExtPubkey ext_key = ExtPubkey(xpub).DerivePubkey(path);
      Pubkey pubkey = ext_key.GetPubkey();

      // Addressクラス生成
      if (arg_type == "sh(wpkh") {
        Script wpkh_script = ScriptUtil::CreateP2wpkhLockingScript(pubkey);
        ByteData160 wpkh_hash = HashUtil::Hash160(wpkh_script);
        pegout_addr = Address(
            pegout_data.net_type, AddressType::kP2shAddress, wpkh_hash);
      } else if (arg_type == "wpkh") {
        pegout_addr =
            Address(pegout_data.net_type, WitnessVersion::kVersion0, pubkey);
      } else {  // if (arg_type == "pkh(")
        // pkh
        pegout_addr = Address(pegout_data.net_type, pubkey);
      }
    } else {
      pegout_addr = pegout_data.btc_address;
    }

    ctxc.AddPegoutTxOut(
        pegout_data.amount, pegout_data.asset, pegout_data.genesisblock_hash,
        pegout_addr, pegout_data.net_type, pegout_data.online_pubkey,
        pegout_data.master_online_key, pegout_data.bitcoin_descriptor,
        pegout_data.bip32_counter, pegout_data.whitelist);
    if (pegout_address != nullptr) {
      *pegout_address = pegout_addr;
    }

  } else {
    ctxc.AddPegoutTxOut(
        pegout_data.amount, pegout_data.asset, pegout_data.genesisblock_hash,
        pegout_data.btc_address);
  }

  // amountが0のfeeは無効と判定
  if (txout_fee.GetConfidentialValue().GetAmount() != 0) {
    ctxc.AddTxOutFee(
        txout_fee.GetConfidentialValue().GetAmount(), txout_fee.GetAsset());
  }

  return ctxc;
}

Privkey ElementsTransactionApi::GetIssuanceBlindingKey(
    const Privkey& master_blinding_key, const Txid& txid, int32_t vout) {
  Privkey blinding_key = ConfidentialTransaction::GetIssuanceBlindingKey(
      master_blinding_key, txid, vout);

  return blinding_key;
}

Amount ElementsTransactionApi::EstimateFee(
    const std::string& tx_hex, const std::vector<ElementsUtxoAndOption>& utxos,
    const ConfidentialAssetId& fee_asset, Amount* tx_fee, Amount* utxo_fee,
    bool is_blind, double effective_fee_rate) const {
  ConfidentialTransactionController txc(tx_hex);

  if (fee_asset.IsEmpty()) {
    warn(CFD_LOG_SOURCE, "Failed to EstimateFee. Empty fee asset.");
    throw CfdException(CfdError::kCfdIllegalArgumentError, "Empty fee asset.");
  }

  // check fee in txout
  bool exist_fee = false;
  const ConfidentialTransaction& ctx = txc.GetTransaction();
  for (const auto& txout : ctx.GetTxOutList()) {
    if (txout.GetLockingScript().IsEmpty()) {
      if (txout.GetAsset().GetHex() != fee_asset.GetHex()) {
        warn(CFD_LOG_SOURCE, "Failed to EstimateFee. Unmatch fee asset.");
        throw CfdException(
            CfdError::kCfdIllegalArgumentError, "Unmatch fee asset.");
      }
      exist_fee = true;
      break;
    }
  }
  if (!exist_fee) {
    txc.AddTxOutFee(Amount::CreateBySatoshiAmount(1), fee_asset);  // dummy fee
  }

  uint32_t size;
  uint32_t witness_size = 0;
  size = txc.GetSizeIgnoreTxIn(is_blind, &witness_size);
  size -= witness_size;
  uint32_t tx_vsize =
      AbstractTransaction::GetVsizeFromSize(size, witness_size);

  size = 0;
  witness_size = 0;
  uint32_t wit_size = 0;
  for (const auto& utxo : utxos) {
    uint32_t pegin_btc_tx_size = 0;
    Script fedpeg_script;
    if (utxo.is_pegin) {
      pegin_btc_tx_size = utxo.pegin_btc_tx_size;
      fedpeg_script = utxo.fedpeg_script;
    }
    // check descriptor
    AddressType addr_type = utxo.utxo.address.GetAddressType();
    // TODO(k-matsuzawa): output descriptorの正式対応後に差し替え
    if (utxo.utxo.address.GetAddress().empty()) {
      if (utxo.utxo.descriptor.find("wpkh(") == 0) {
        addr_type = AddressType::kP2wpkhAddress;
      } else if (utxo.utxo.descriptor.find("wsh(") == 0) {
        addr_type = AddressType::kP2wshAddress;
      } else if (utxo.utxo.descriptor.find("pkh(") == 0) {
        addr_type = AddressType::kP2pkhAddress;
      } else if (utxo.utxo.descriptor.find("sh(") == 0) {
        addr_type = AddressType::kP2shAddress;
      }
    }
    if (utxo.utxo.descriptor.find("sh(wpkh(") == 0) {
      addr_type = AddressType::kP2shP2wpkhAddress;
    } else if (utxo.utxo.descriptor.find("sh(wsh(") == 0) {
      addr_type = AddressType::kP2shP2wshAddress;
    }

    uint32_t txin_size = ConfidentialTxIn::EstimateTxInSize(
        addr_type, utxo.utxo.redeem_script, pegin_btc_tx_size, fedpeg_script,
        utxo.is_issuance, utxo.is_blind_issuance, &wit_size);
    txin_size -= wit_size;
    size += txin_size;
    witness_size += wit_size;
  }
  uint32_t utxo_vsize =
      AbstractTransaction::GetVsizeFromSize(size, witness_size);

  uint64_t fee_rate = static_cast<uint64_t>(floor(effective_fee_rate * 1000));
  FeeCalculator fee_calc(fee_rate);
  Amount tx_fee_amount = fee_calc.GetFee(tx_vsize);
  Amount utxo_fee_amount = fee_calc.GetFee(utxo_vsize);
  Amount fee = tx_fee_amount + utxo_fee_amount;

  if (tx_fee) *tx_fee = tx_fee_amount;
  if (utxo_fee) *utxo_fee = utxo_fee_amount;

  info(
      CFD_LOG_SOURCE, "EstimateFee rate={} fee={} tx={} utxo={}",
      effective_fee_rate, fee.GetSatoshiValue(),
      tx_fee_amount.GetSatoshiValue(), utxo_fee_amount.GetSatoshiValue());
  return fee;
}

}  // namespace api
}  // namespace cfd

#endif  // CFD_DISABLE_ELEMENTS
