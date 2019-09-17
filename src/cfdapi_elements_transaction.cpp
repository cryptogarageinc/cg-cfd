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

#include "cfd/cfd_elements_address.h"
#include "cfd/cfdapi_address.h"
#include "cfd/cfdapi_elements_transaction.h"
#include "cfd/cfdapi_struct.h"
#include "cfd/cfdapi_transaction.h"
#include "cfdapi_internal.h"  // NOLINT

namespace cfd {
namespace api {

using cfd::ConfidentialTransactionController;
using cfd::ElementsAddressUtil;
using cfd::api::AddressApi;
using cfd::api::TransactionApi;
using cfdcore::AbstractElementsAddress;
using cfdcore::AddressType;
using cfdcore::Amount;
using cfdcore::BlindFactor;
using cfdcore::BlindParameter;
using cfdcore::BlockHash;
using cfdcore::ByteData;
using cfdcore::ByteData256;
using cfdcore::CfdError;
using cfdcore::CfdException;
using cfdcore::ConfidentialAssetId;
using cfdcore::ConfidentialTransaction;
using cfdcore::ConfidentialValue;
using cfdcore::ElementsConfidentialAddress;
using cfdcore::ElementsUnblindedAddress;
using cfdcore::IssuanceBlindingKeyPair;
using cfdcore::IssuanceParameter;
using cfdcore::Privkey;
using cfdcore::Pubkey;
using cfdcore::Script;
using cfdcore::ScriptBuilder;
using cfdcore::ScriptOperator;
using cfdcore::SigHashType;
using cfdcore::Transaction;
using cfdcore::Txid;
using cfdcore::UnblindParameter;
using cfdcore::logger::info;
using cfdcore::logger::warn;

// -----------------------------------------------------------------------------
// ファイル内関数
// -----------------------------------------------------------------------------
/**
 * @brief Elements用AddMultisigSignのリクエスト情報チェックを行います。
 * @param[in] req       リクエスト情報
 * @param[in] addr_type アドレス種別
 */
static void ValidateAddMultisigSignRequestElements(  // linefeed
    AddMultisigSignRequestStruct req, AddressType addr_type) {
  // check txHex
  if (req.tx_hex.empty()) {
    warn(
        CFD_LOG_SOURCE,
        "Failed to AddSegwitMultisigSignRequest. Transaction hex empty.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Invalid hex string. empty txHex.");
  }

  // check require script
  switch (addr_type) {
    case AddressType::kP2shAddress: {
      if (req.redeem_script.empty()) {
        warn(
            CFD_LOG_SOURCE,
            "Failed to AddSegwitMultisigSignRequest. redeem script empty.");
        throw CfdException(
            CfdError::kCfdIllegalArgumentError,
            "Invalid hex string. empty redeemScript.");
      }
      break;
    }
    case AddressType::kP2wshAddress: {
      // TODO(MariSoejima): Elements+Bech32現状対応なし
      throw CfdException(
          CfdError::kCfdOutOfRangeError,
          "Failed to AddMultisigSign. p2wsh is excluded.");

      //      if (req.witness_script.empty()) {
      //        warn(CFD_LOG_SOURCE,
      //             "Failed to AddSegwitMultisigSignRequest. witness script
      //             empty.");
      //        throw CfdException(CfdError::kCfdIllegalArgumentError,
      //                           "Invalid hex string. empty witnessScript.");
      //      }
      //
      break;
    }
    case AddressType::kP2shP2wshAddress: {
      if (req.redeem_script.empty()) {
        warn(
            CFD_LOG_SOURCE,
            "Failed to AddSegwitMultisigSignRequest. redeem script empty.");
        throw CfdException(
            CfdError::kCfdIllegalArgumentError,
            "Invalid hex string. empty redeemScript.");
      }
      if (req.witness_script.empty()) {
        warn(
            CFD_LOG_SOURCE,
            "Failed to AddSegwitMultisigSignRequest. witness script empty.");
        throw CfdException(
            CfdError::kCfdIllegalArgumentError,
            "Invalid hex string. empty witnessScript.");
      }
      break;
    }
    default: {
      warn(
          CFD_LOG_SOURCE,
          "Failed to AddSegwitMultisigSignRequest. address type must be one "
          "of p2sh address.");
      throw CfdException(
          CfdError::kCfdIllegalArgumentError, "Invalid address type.");
    }
  }

  // check signData (not empty)
  if (req.sign_params.empty()) {
    warn(
        CFD_LOG_SOURCE,
        "Failed to AddSegwitMultisigSignRequest. sign parameters empty.");
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Invalid array length. empty signParams.");
  }

  // check signData (too much data)
  if (req.sign_params.size() > 15) {
    warn(
        CFD_LOG_SOURCE,
        "Failed to AddSegwitMultisigSignRequest. sign array length over.");
    throw CfdException(
        CfdError::kCfdOutOfRangeError,
        "Value out of range. sign array length over.");
  }
}

ElementsCreateRawTransactionResponseStruct
ElementsTransactionApi::CreateRawTransaction(  // NOLINT
    const ElementsCreateRawTransactionRequestStruct& request) {
  auto call_func = [](const ElementsCreateRawTransactionRequestStruct& request)
      -> ElementsCreateRawTransactionResponseStruct {  // NOLINT
    ElementsCreateRawTransactionResponseStruct response;
    // Transaction作成
    ConfidentialTransactionController ctxc(request.version, request.locktime);

    // TxInの追加
    const uint32_t kLockTimeDisabledSequence =
        ctxc.GetLockTimeDisabledSequence();
    for (ElementsTxInRequestStruct txin_req : request.txins) {
      // TxInのunlocking_scriptは空で作成
      if (kLockTimeDisabledSequence == txin_req.sequence) {
        ctxc.AddTxIn(
            Txid(txin_req.txid), txin_req.vout, ctxc.GetDefaultSequence());
      } else {
        ctxc.AddTxIn(Txid(txin_req.txid), txin_req.vout, txin_req.sequence);
      }
    }

    // TxOutの追加
    for (ElementsTxOutRequestStruct txout_req : request.txouts) {
      const std::string addr = txout_req.address;
      if (AbstractElementsAddress::IsConfidentialAddress(addr)) {
        ctxc.AddTxOut(
            ElementsConfidentialAddress(addr),
            Amount::CreateBySatoshiAmount(txout_req.amount),
            ConfidentialAssetId(txout_req.asset), txout_req.is_remove_nonce);
      } else {
        ctxc.AddTxOut(
            ElementsUnblindedAddress(addr),
            Amount::CreateBySatoshiAmount(txout_req.amount),
            ConfidentialAssetId(txout_req.asset), txout_req.is_remove_nonce);
      }
    }

    // feeの追加
    ElementsTxOutFeeRequestStruct fee_req = request.fee;
    // amountが0のfeeは無効と判定
    if (fee_req.amount != 0) {
      ctxc.AddTxOutFee(
          Amount::CreateBySatoshiAmount(fee_req.amount),
          ConfidentialAssetId(fee_req.asset));
    }

    response.hex = ctxc.GetHex();
    return response;
  };

  ElementsCreateRawTransactionResponseStruct result;
  result = ExecuteStructApi<
      ElementsCreateRawTransactionRequestStruct,
      ElementsCreateRawTransactionResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

GetWitnessStackNumResponseStruct ElementsTransactionApi::GetWitnessStackNum(
    const GetWitnessStackNumRequestStruct& request) {
  auto call_func = [](const GetWitnessStackNumRequestStruct& request)
      -> GetWitnessStackNumResponseStruct {  // NOLINT
    GetWitnessStackNumResponseStruct response;
    const std::string& hex_string = request.tx_hex;
    if (hex_string.empty()) {
      warn(CFD_LOG_SOURCE, "Failed to GetWitnessStackNum. hex empty.");
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Invalid hex string. empty data.");
    }

    // TransactionController作成
    ConfidentialTransactionController txc(hex_string);

    uint32_t count =
        txc.GetWitnessStackNum(Txid(request.txin_txid), request.txin_vout);

    response.count = count;
    return response;
  };

  GetWitnessStackNumResponseStruct result;
  result = ExecuteStructApi<
      GetWitnessStackNumRequestStruct, GetWitnessStackNumResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

AddSignResponseStruct ElementsTransactionApi::AddSign(
    const AddSignRequestStruct& request) {
  auto call_func =
      [](const AddSignRequestStruct& request) -> AddSignResponseStruct {
    AddSignResponseStruct response;
    const std::string& hex_string = request.tx_hex;
    if (hex_string.empty()) {
      warn(CFD_LOG_SOURCE, "Failed to AddSignRequest. hex empty.");
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Invalid hex string. empty data.");
    }

    // TransactionController作成
    ConfidentialTransactionController txc(hex_string);

    if (request.is_witness) {
      // Witnessの追加
      Txid txid = Txid(request.txin_txid);
      if (request.clear_stack) {
        txc.RemoveWitnessStackAll(txid, request.txin_vout);
      }
      std::vector<ByteData> witness_datas;
      for (const SignDataStruct& stack_req : request.sign_param) {
        ByteData byte_data = TransactionApi::ConvertSignDataToSignature(
            stack_req.hex, (stack_req.type == "sign"), stack_req.der_encode,
            stack_req.sighash_type, stack_req.sighash_anyone_can_pay);
        witness_datas.push_back(byte_data);
      }
      txc.AddWitnessStack(txid, request.txin_vout, witness_datas);
    } else {
      std::vector<ByteData> unlock_script;
      for (const SignDataStruct& stack_req : request.sign_param) {
        ByteData byte_data = TransactionApi::ConvertSignDataToSignature(
            stack_req.hex, (stack_req.type == "sign"), stack_req.der_encode,
            stack_req.sighash_type, stack_req.sighash_anyone_can_pay);
        unlock_script.push_back(byte_data);
      }
      txc.SetUnlockingScript(
          Txid(request.txin_txid), request.txin_vout, unlock_script);
    }

    response.hex = txc.GetHex();
    return response;
  };

  AddSignResponseStruct result;
  result = ExecuteStructApi<AddSignRequestStruct, AddSignResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

AddMultisigSignResponseStruct ElementsTransactionApi::AddMultisigSign(
    const AddMultisigSignRequestStruct& request) {
  auto call_func = [](const AddMultisigSignRequestStruct& request)
      -> AddMultisigSignResponseStruct {  // NOLINT
    AddMultisigSignResponseStruct response;
    // レスポンスとなるモデルへ変換
    // validate request
    AddressType addr_type = AddressApi::ConvertAddressType(request.txin_type);
    ValidateAddMultisigSignRequestElements(request, addr_type);

    const std::string& hex_string = request.tx_hex;
    warn(CFD_LOG_SOURCE, "hex:{}", hex_string);
    ConfidentialTransactionController txc(hex_string);

    // extract pubkeys from redeem script
    Script redeem_script = addr_type == AddressType::kP2shAddress
                               ? Script(request.redeem_script)
                               : Script(request.witness_script);

    std::vector<Pubkey> pubkeys =
        TransactionApi::ExtractPubkeysFromMultisigScript(redeem_script);
    // get signParams from json request
    std::vector<MultisigSignDataStruct> sign_params = request.sign_params;

    // set signParam to signature_data (only contains relatedPubkey);
    std::vector<ByteData> signature_data;
    for (const Pubkey& pubkey : pubkeys) {
      for (auto itr = sign_params.begin(); itr != sign_params.end();) {
        MultisigSignDataStruct sign_param = *itr;
        if (sign_param.related_pubkey != pubkey.GetHex()) {
          ++itr;
          continue;
        }

        itr = sign_params.erase(itr);
        ByteData byte_data = TransactionApi::ConvertSignDataToSignature(
            sign_param.hex, true, sign_param.der_encode,
            sign_param.sighash_type, sign_param.sighash_anyone_can_pay);
        signature_data.push_back(byte_data);
      }
    }

    // set the others to signature data
    for (MultisigSignDataStruct sign_param : sign_params) {
      // related pubkey not found in script
      if (!sign_param.related_pubkey.empty()) {
        warn(
            CFD_LOG_SOURCE,
            "Failed to AddMultisigSign. Missing related pubkey"
            " in script.: relatedPubkey={}, script={}",
            sign_param.related_pubkey, redeem_script.GetHex());
        throw CfdException(
            CfdError::kCfdIllegalArgumentError,
            "Missing related pubkey in script."
            " Check your signature and pubkey pair.");
      }
      ByteData byte_data = TransactionApi::ConvertSignDataToSignature(
          sign_param.hex, true, sign_param.der_encode, sign_param.sighash_type,
          sign_param.sighash_anyone_can_pay);
      signature_data.push_back(byte_data);
    }

    // set signatures to target input
    switch (addr_type) {
      case AddressType::kP2shAddress: {
        // non-segwit
        ScriptBuilder sb;
        sb.AppendOperator(ScriptOperator::OP_0);
        for (const ByteData& signature : signature_data) {
          sb.AppendData(signature);
        }
        sb.AppendData(redeem_script);

        txc.SetUnlockingScript(
            Txid(request.txin_txid), request.txin_vout, sb.Build());
        break;
      }
      case AddressType::kP2wshAddress:
        // 対象外
        break;
      case AddressType::kP2shP2wshAddress: {
        Txid txid = Txid(request.txin_txid);
        if (request.clear_stack) {
          txc.RemoveWitnessStackAll(txid, request.txin_vout);
        }
        // set empty data
        std::vector<ByteData> witness_stack;
        // push empty byte
        witness_stack.push_back(ByteData());
        // create stack
        std::copy(
            signature_data.begin(), signature_data.end(),
            std::back_inserter(witness_stack));
        witness_stack.push_back(redeem_script.GetData());

        // set witness stack
        txc.AddWitnessStack(txid, request.txin_vout, witness_stack);
        break;
      }
      default:
        // Unreachable, validate method ensures correct address type.
        break;
    }

    if (addr_type == AddressType::kP2shP2wshAddress) {
      // set p2sh redeem script to unlockking script
      Script p2sh_redeem_script(request.redeem_script);
      ScriptBuilder sb;
      sb.AppendData(p2sh_redeem_script);
      txc.SetUnlockingScript(
          Txid(request.txin_txid), request.txin_vout, sb.Build());
    }

    response.hex = txc.GetHex();
    return response;
  };

  AddMultisigSignResponseStruct result;
  result = ExecuteStructApi<
      AddMultisigSignRequestStruct, AddMultisigSignResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

UpdateWitnessStackResponseStruct ElementsTransactionApi::UpdateWitnessStack(
    const UpdateWitnessStackRequestStruct& request) {
  auto call_func = [](const UpdateWitnessStackRequestStruct& request)
      -> UpdateWitnessStackResponseStruct {  // NOLINT
    UpdateWitnessStackResponseStruct response;
    const std::string& hex_string = request.tx_hex;
    if (hex_string.empty()) {
      warn(CFD_LOG_SOURCE, "Failed to UpdateWitnessStack. hex empty.");
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Invalid hex string. empty data.");
    }

    // TransactionController作成
    ConfidentialTransactionController txc(hex_string);

    // Witnessの更新
    const WitnessStackDataStruct& stack_req = request.witness_stack;
    ByteData byte_data = TransactionApi::ConvertSignDataToSignature(
        stack_req.hex, (stack_req.type == "sign"), stack_req.der_encode,
        stack_req.sighash_type, stack_req.sighash_anyone_can_pay);
    txc.SetWitnessStack(
        Txid(request.txin_txid), request.txin_vout,
        static_cast<uint32_t>(stack_req.index), byte_data);

    response.hex = txc.GetHex();
    return response;
  };

  UpdateWitnessStackResponseStruct result;
  result = ExecuteStructApi<
      UpdateWitnessStackRequestStruct, UpdateWitnessStackResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

CreateElementsSignatureHashResponseStruct
ElementsTransactionApi::CreateSignatureHash(  // NOLINT
    const CreateElementsSignatureHashRequestStruct& request) {
  auto call_func = [](const CreateElementsSignatureHashRequestStruct& request)
      -> CreateElementsSignatureHashResponseStruct {  // NOLINT
    CreateElementsSignatureHashResponseStruct response;
    std::string sig_hash;
    int64_t amount = request.amount;
    const std::string& hashtype_str = request.hash_type;
    const std::string& pubkey_hex = request.pubkey_hex;
    const std::string& script_hex = request.script_hex;
    const std::string& value_hex = request.confidential_value_hex;
    const Txid& txid = Txid(request.txin_txid);
    uint32_t vout = request.txin_vout;
    ConfidentialTransactionController txc(request.tx_hex);
    SigHashType sighashtype = TransactionApi::ConvertSigHashType(
        request.sighash_type, request.sighash_anyone_can_pay);

    if ((hashtype_str == "p2pkh") || (hashtype_str == "p2wpkh")) {
      if (value_hex.empty()) {
        sig_hash = txc.CreateSignatureHash(
            txid, vout, Pubkey(pubkey_hex), sighashtype,
            Amount::CreateBySatoshiAmount(amount), (hashtype_str == "p2wpkh"));
      } else {
        sig_hash = txc.CreateSignatureHash(
            txid, vout, Pubkey(pubkey_hex), sighashtype, ByteData(value_hex),
            (hashtype_str == "p2wpkh"));
      }
    } else if ((hashtype_str == "p2sh") || (hashtype_str == "p2wsh")) {
      if (value_hex.empty()) {
        sig_hash = txc.CreateSignatureHash(
            txid, vout, Script(script_hex), sighashtype,
            Amount::CreateBySatoshiAmount(amount), (hashtype_str == "p2wsh"));
      } else {
        sig_hash = txc.CreateSignatureHash(
            txid, vout, Script(script_hex), sighashtype, ByteData(value_hex),
            (hashtype_str == "p2wsh"));
      }
    } else {
      warn(
          CFD_LOG_SOURCE,
          "Failed to CreateSignatureHash. Invalid hashtype_str:  "
          "hashtype_str={}",  // NOLINT
          hashtype_str);
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Invalid hashtype_str. hashtype_str must be \"p2pkh\" "
          "or \"p2sh\" or \"p2wpkh\" or \"p2wsh\".");  // NOLINT
    }

    response.sighash = sig_hash;
    return response;
  };

  CreateElementsSignatureHashResponseStruct result;
  result = ExecuteStructApi<
      CreateElementsSignatureHashRequestStruct,
      CreateElementsSignatureHashResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

BlindRawTransactionResponseStruct ElementsTransactionApi::BlindTransaction(
    const BlindRawTransactionRequestStruct& request) {
  auto call_func = [](const BlindRawTransactionRequestStruct& request)
      -> BlindRawTransactionResponseStruct {  // NOLINT
    BlindRawTransactionResponseStruct response;
    ConfidentialTransactionController txc(request.tx_hex);
    const ConfidentialTransaction& tx = txc.GetTransaction();

    uint32_t txin_count = tx.GetTxInCount();
    uint32_t txout_count = tx.GetTxOutCount();
    const std::vector<BlindTxInRequestStruct>& txins = request.txins;
    const std::vector<std::string>& pubkeys = request.blind_pubkeys;
    if (txins.size() == 0) {
      warn(CFD_LOG_SOURCE, "Failed to txins empty.");
      throw CfdException(
          CfdError::kCfdOutOfRangeError, "JSON value error. Empty txins.");
    }
    if (txins.size() != txin_count) {
      warn(
          CFD_LOG_SOURCE, "Failed to txins count. {} != {}", txins.size(),
          txin_count);
      throw CfdException(
          CfdError::kCfdOutOfRangeError,
          "JSON value error. Unmatch txins count.");
    }
    if (txout_count == 0) {
      warn(CFD_LOG_SOURCE, "Failed to txouts empty.");
      throw CfdException(
          CfdError::kCfdOutOfRangeError, "JSON value error. txouts empty.");
    }

    // fee count
    uint32_t fee_count = 0;
    uint32_t fee_index = 0;
    std::set<uint32_t> fee_indexes;
    for (const auto& txout : tx.GetTxOutList()) {
      if (txout.GetLockingScript().IsEmpty()) {
        ++fee_count;
        fee_indexes.insert(fee_index);
      }
      ++fee_index;
    }
    if (fee_count == txout_count) {
      warn(CFD_LOG_SOURCE, "Failed to txouts fee only.");
      throw CfdException(
          CfdError::kCfdOutOfRangeError, "JSON value error. txouts fee only.");
    }
    uint32_t txout_value_count = txout_count - fee_count;
    if (pubkeys.size() < txout_value_count) {
      warn(
          CFD_LOG_SOURCE, "Failed to pubkey count. {} < {}", pubkeys.size(),
          txout_value_count);
      throw CfdException(
          CfdError::kCfdOutOfRangeError,
          "JSON value error. Pubkey count not enough.");
    }
    bool is_insert_fee_key = false;
    if ((!fee_indexes.empty()) && (pubkeys.size() == txout_value_count)) {
      // insert empty key (for fee)
      is_insert_fee_key = true;
    }

    std::vector<BlindParameter> params(txin_count);
    std::vector<bool> exist_list(txin_count, false);
    std::vector<Pubkey> blind_pubkeys(txout_count);
    std::vector<IssuanceBlindingKeyPair> key_pairs;
    uint32_t index;

    // BlindingPubkey
    uint32_t pubkey_index = 0;
    for (index = 0; index < txout_count; ++index) {
      if (is_insert_fee_key && (fee_indexes.count(index) > 0)) {
        // throuth
      } else if (pubkeys.size() > pubkey_index) {
        if (!pubkeys[pubkey_index].empty()) {
          blind_pubkeys[index] = Pubkey(pubkeys[pubkey_index]);
        }
        ++pubkey_index;
      }
    }

    // BlindParameter
    uint32_t offset = 0;
    uint32_t count = 0;
    for (const auto& txin : txins) {
      if (!txin.txid.empty()) {
        // index指定
        offset = tx.GetTxInIndex(Txid(txin.txid), txin.vout);
        exist_list[offset] = true;
        params[offset].asset = ConfidentialAssetId(txin.asset);
        params[offset].vbf = BlindFactor(txin.blind_factor);
        params[offset].abf = BlindFactor(txin.asset_blind_factor);
        params[offset].value =
            ConfidentialValue(Amount::CreateBySatoshiAmount(txin.amount));
        ++count;
      }
    }

    if (count != txin_count) {
      for (const auto& txin : txins) {
        if (txin.txid.empty()) {
          // 指定なしなら前方から挿入
          for (index = 0; index < txin_count; ++index) {
            if (!exist_list[index]) {
              exist_list[index] = true;
              params[index].asset = ConfidentialAssetId(txin.asset);
              params[index].vbf = BlindFactor(txin.blind_factor);
              params[index].abf = BlindFactor(txin.asset_blind_factor);
              params[index].value = ConfidentialValue(
                  Amount::CreateBySatoshiAmount(txin.amount));
              break;
            }
          }
        }
      }
    }

    // Issuance
    if (request.issuances.size() > 0) {
      // txinの個数分確保
      key_pairs.resize(txin_count);

      for (BlindIssuanceRequestStruct issuance : request.issuances) {
        uint32_t index = txc.GetTransaction().GetTxInIndex(
            Txid(issuance.txid), issuance.vout);
        IssuanceBlindingKeyPair key;
        if (!issuance.asset_blinding_key.empty()) {
          key.asset_key = Privkey(issuance.asset_blinding_key);
        }
        if (!issuance.token_blinding_key.empty()) {
          key.token_key = Privkey(issuance.token_blinding_key);
        }
        key_pairs[index] = key;
      }
    }

    txc.BlindTransaction(params, key_pairs, blind_pubkeys);
    response.hex = txc.GetHex();
    return response;
  };

  BlindRawTransactionResponseStruct result;
  result = ExecuteStructApi<
      BlindRawTransactionRequestStruct, BlindRawTransactionResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

UnblindRawTransactionResponseStruct ElementsTransactionApi::UnblindTransaction(
    const UnblindRawTransactionRequestStruct& request) {
  auto call_func = [](const UnblindRawTransactionRequestStruct& request)
      -> UnblindRawTransactionResponseStruct {
    UnblindRawTransactionResponseStruct response;
    ConfidentialTransactionController ctxc(request.tx_hex);
    const ConfidentialTransaction& tx = ctxc.GetTransaction();

    bool unblind_single = false;
    // fee count
    uint32_t fee_count = 0;
    uint32_t fee_index = 0;
    std::set<uint32_t> fee_indexes;
    for (const auto& txout : tx.GetTxOutList()) {
      if (txout.GetLockingScript().IsEmpty()) {
        ++fee_count;
        fee_indexes.insert(fee_index);
      }
      ++fee_index;
    }
    bool is_insert_fee_key = false;
    uint32_t txout_count = tx.GetTxOutCount();
    const std::vector<std::string>& blinding_keys = request.blinding_keys;

    int64_t target_output_index = request.target_output_index;
    if (target_output_index >= 0) {
      if (target_output_index > std::numeric_limits<uint32_t>::max()) {
        warn(
            CFD_LOG_SOURCE, "Invalid txout index. : target_output_index={}",
            target_output_index);
        throw CfdException(
            CfdError::kCfdOutOfRangeError,
            "target txout index error. Value out of range.");
      }

      if (blinding_keys.size() != 1) {
        warn(
            CFD_LOG_SOURCE,
            "blindingKeys size is unmatch"
            " target txout size.");
        throw CfdException(
            CfdError::kCfdIllegalArgumentError,
            "JSON value error. If set targetOutputIndex,"
            " blindingKeys must set one blinding key.");
      }
      unblind_single = true;
    } else {
      uint32_t txout_value_count = txout_count - fee_count;
      if (txout_value_count > blinding_keys.size()) {
        // TxOutの数とBlindingkeyの数が不一致の場合はエラー
        warn(CFD_LOG_SOURCE, "blindingKeys size is unmatch txout size.");
        throw CfdException(
            CfdError::kCfdIllegalArgumentError,
            "JSON value error. blindingKeys size is unmatch txout size.");
      }

      if ((!fee_indexes.empty()) &&
          (blinding_keys.size() == txout_value_count)) {
        // insert empty key (for fee)
        is_insert_fee_key = true;
      }
    }

    std::vector<Privkey> privkeys(txout_count);
    uint32_t blindingkey_index = 0;
    for (uint32_t index = 0; index < txout_count; ++index) {
      if (is_insert_fee_key && (fee_indexes.count(index) > 0)) {
        // throuth
      } else if (blinding_keys.size() > blindingkey_index) {
        if (!blinding_keys[blindingkey_index].empty()) {
          privkeys[index] = Privkey(blinding_keys[blindingkey_index]);
        }
        ++blindingkey_index;
      }
    }

    // TxOutをUnblind
    std::vector<UnblindParameter> unblind_params;
    if (unblind_single) {
      const Privkey blinding_key(privkeys[0]);
      unblind_params.push_back(
          ctxc.UnblindTxOut(target_output_index, blinding_key));
    } else {
      unblind_params = ctxc.UnblindTransaction(privkeys);
    }
    for (UnblindParameter unblind_param : unblind_params) {
      UnblindOutputStruct output;
      if (!unblind_param.asset.GetHex().empty()) {
        output.asset = unblind_param.asset.GetHex();
        output.blind_factor = unblind_param.vbf.GetHex();
        output.asset_blind_factor = unblind_param.abf.GetHex();
        output.amount = unblind_param.value.GetAmount().GetSatoshiValue();
      }
      response.outputs.push_back(output);
    }

    // TxIn Unblind(Issuanceの場合)
    if (request.issuances.size() != 0) {
      for (UnblindIssuanceStruct issuance : request.issuances) {
        uint32_t txin_index =
            tx.GetTxInIndex(Txid(issuance.txid), issuance.vout);
        bool is_find = false;
        UnblindIssuanceOutputStruct output;
        Privkey asset_blinding_key;
        Privkey token_blinding_key;
        if (!issuance.asset_blinding_key.empty()) {
          asset_blinding_key = Privkey(issuance.asset_blinding_key);
          is_find = true;
        }
        if (!issuance.token_blinding_key.empty()) {
          token_blinding_key = Privkey(issuance.token_blinding_key);
          is_find = true;
        }
        if (is_find) {
          std::vector<UnblindParameter> issuance_outputs =
              ctxc.UnblindIssuance(
                  txin_index, asset_blinding_key, token_blinding_key);

          output.txid = issuance.txid;
          output.vout = issuance.vout;
          output.asset = issuance_outputs[0].asset.GetHex();
          output.assetamount =
              issuance_outputs[0].value.GetAmount().GetSatoshiValue();
          if (issuance_outputs.size() > 1) {
            output.token = issuance_outputs[1].asset.GetHex();
            output.tokenamount =
                issuance_outputs[1].value.GetAmount().GetSatoshiValue();
          }
        }
        response.issuance_outputs.push_back(output);
      }
    }
    response.hex = ctxc.GetHex();
    return response;
  };

  UnblindRawTransactionResponseStruct result;
  result = ExecuteStructApi<
      UnblindRawTransactionRequestStruct, UnblindRawTransactionResponseStruct>(
      request, call_func, std::string(__FUNCTION__));

  return result;
}

SetRawIssueAssetResponseStruct ElementsTransactionApi::SetRawIssueAsset(
    const SetRawIssueAssetRequestStruct& request) {
  auto call_func = [](const SetRawIssueAssetRequestStruct& request)
      -> SetRawIssueAssetResponseStruct {  // NOLINT
    SetRawIssueAssetResponseStruct response;
    ConfidentialTransactionController ctxc(request.tx_hex);

    for (IssuanceDataRequestStruct req_issuance : request.issuances) {
      // Txin1つずつissuanceの設定を行う
      IssuanceParameter issuance_param = ctxc.SetAssetIssuance(
          Txid(req_issuance.txin_txid), req_issuance.txin_vout,
          Amount::CreateBySatoshiAmount(req_issuance.asset_amount),
          ElementsAddressUtil::GetElementsAddress(req_issuance.asset_address),
          Amount::CreateBySatoshiAmount(req_issuance.token_amount),
          ElementsAddressUtil::GetElementsAddress(req_issuance.token_address),
          req_issuance.is_blind, ByteData256(req_issuance.contract_hash),
          false, req_issuance.is_remove_nonce);

      IssuanceDataResponseStruct res_issuance;
      res_issuance.txin_txid = req_issuance.txin_txid;
      res_issuance.txin_vout = req_issuance.txin_vout;
      res_issuance.asset = issuance_param.asset.GetHex();
      res_issuance.entropy = issuance_param.entropy.GetHex();
      res_issuance.token = issuance_param.token.GetHex();

      response.issuances.push_back(res_issuance);
    }

    // すべて設定後にTxoutのRandomize
    if (request.is_randomize) {
      ctxc.RandomizeTxOut();
    }
    response.hex = ctxc.GetHex();
    return response;
  };

  SetRawIssueAssetResponseStruct result;
  result = ExecuteStructApi<
      SetRawIssueAssetRequestStruct, SetRawIssueAssetResponseStruct>(
      request, call_func, std::string(__FUNCTION__));

  return result;
}

SetRawReissueAssetResponseStruct ElementsTransactionApi::SetRawReissueAsset(
    const SetRawReissueAssetRequestStruct& request) {
  auto call_func = [](const SetRawReissueAssetRequestStruct& request)
      -> SetRawReissueAssetResponseStruct {  // NOLINT
    SetRawReissueAssetResponseStruct response;
    ConfidentialTransactionController ctxc(request.tx_hex);

    for (ReissuanceDataRequestStruct req_issuance : request.issuances) {
      // Txin1つずつissuanceの設定を行う
      IssuanceParameter issuance_param = ctxc.SetAssetReissuance(
          Txid(req_issuance.txin_txid), req_issuance.txin_vout,
          Amount::CreateBySatoshiAmount(req_issuance.amount),
          ElementsAddressUtil::GetElementsAddress(req_issuance.address),
          BlindFactor(req_issuance.asset_blinding_nonce),
          BlindFactor(req_issuance.asset_entropy), false,
          req_issuance.is_remove_nonce);

      ReissuanceDataResponseStruct res_issuance;
      res_issuance.txin_txid = req_issuance.txin_txid;
      res_issuance.txin_vout = req_issuance.txin_vout;
      res_issuance.asset = issuance_param.asset.GetHex();
      res_issuance.entropy = issuance_param.entropy.GetHex();
      response.issuances.push_back(res_issuance);
    }

    // すべて設定後にTxoutのRandomize
    if (request.is_randomize) {
      ctxc.RandomizeTxOut();
    }
    response.hex = ctxc.GetHex();
    return response;
  };

  SetRawReissueAssetResponseStruct result;
  result = ExecuteStructApi<
      SetRawReissueAssetRequestStruct, SetRawReissueAssetResponseStruct>(
      request, call_func, std::string(__FUNCTION__));

  return result;
}

ElementsCreateRawPeginResponseStruct
ElementsTransactionApi::CreateRawPeginTransaction(  // NOLINT
    const ElementsCreateRawPeginRequestStruct& request) {
  auto call_func = [](const ElementsCreateRawPeginRequestStruct& request)
      -> ElementsCreateRawPeginResponseStruct {  // NOLINT
    ElementsCreateRawPeginResponseStruct response;
    // Transaction作成
    ConfidentialTransactionController ctxc(request.version, request.locktime);

    // TxInの追加
    const uint32_t kLockTimeDisabledSequence =
        ctxc.GetLockTimeDisabledSequence();
    for (ElementsPeginTxInStruct txin_req : request.txins) {
      Txid txid(txin_req.txid);
      uint32_t vout = txin_req.vout;

      // TxInのunlocking_scriptは空で作成
      if (kLockTimeDisabledSequence == txin_req.sequence) {
        ctxc.AddTxIn(txid, vout, ctxc.GetDefaultSequence());
      } else {
        ctxc.AddTxIn(txid, vout, txin_req.sequence);
      }

      // PeginWitnessの追加
      if (txin_req.is_pegin) {
        info(
            CFD_LOG_SOURCE, "rm btcWitness[{}]",
            txin_req.is_remove_mainchain_tx_witness);
        ByteData mainchain_tx =
            ByteData(txin_req.peginwitness.mainchain_raw_transaction);
        mainchain_tx = ConfidentialTransaction::GetBitcoinTransaction(
            mainchain_tx, txin_req.is_remove_mainchain_tx_witness);

        ctxc.AddPeginWitness(
            txid, vout,
            Amount::CreateBySatoshiAmount(txin_req.peginwitness.amount),
            ConfidentialAssetId(txin_req.peginwitness.asset),
            BlockHash(txin_req.peginwitness.mainchain_genesis_block_hash),
            Script(txin_req.peginwitness.claim_script), mainchain_tx,
            ByteData(txin_req.peginwitness.mainchain_txoutproof));
      }
    }

    // TxOutの追加
    for (ElementsPeginTxOutStruct txout_req : request.txouts) {
      const std::string addr = txout_req.address;
      if (AbstractElementsAddress::IsConfidentialAddress(addr)) {
        ctxc.AddTxOut(
            ElementsConfidentialAddress(addr),
            Amount::CreateBySatoshiAmount(txout_req.amount),
            ConfidentialAssetId(txout_req.asset), txout_req.is_remove_nonce);
      } else {
        ctxc.AddTxOut(
            ElementsUnblindedAddress(addr),
            Amount::CreateBySatoshiAmount(txout_req.amount),
            ConfidentialAssetId(txout_req.asset), txout_req.is_remove_nonce);
      }
    }

    // feeの追加
    ElementsPeginTxOutFeeStruct fee_req = request.fee;
    ctxc.AddTxOutFee(
        Amount::CreateBySatoshiAmount(fee_req.amount),
        ConfidentialAssetId(fee_req.asset));

    response.hex = ctxc.GetHex();
    return response;
  };

  ElementsCreateRawPeginResponseStruct result;
  result = ExecuteStructApi<
      ElementsCreateRawPeginRequestStruct,
      ElementsCreateRawPeginResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

GetIssuanceBlindingKeyResponseStruct
ElementsTransactionApi::GetIssuanceBlindingKey(
    const GetIssuanceBlindingKeyRequestStruct& request) {
  auto call_func = [](const GetIssuanceBlindingKeyRequestStruct& request)
      -> GetIssuanceBlindingKeyResponseStruct {  // NOLINT
    GetIssuanceBlindingKeyResponseStruct response;
    Privkey blinding_key = ConfidentialTransaction::GetIssuanceBlindingKey(
        Privkey(request.master_blinding_key), Txid(request.txid),
        request.vout);
    response.blinding_key = blinding_key.GetHex();
    return response;
  };

  GetIssuanceBlindingKeyResponseStruct result;
  result = ExecuteStructApi<
      GetIssuanceBlindingKeyRequestStruct,
      GetIssuanceBlindingKeyResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

}  // namespace api
}  // namespace cfd
#endif  // CFD_DISABLE_ELEMENTS
