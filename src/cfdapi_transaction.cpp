// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_transaction.cpp
 *
 * @brief cfd-apiで利用するTransaction作成の実装ファイル
 */

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

#include "cfd/cfd_address.h"
#include "cfd/cfd_script.h"
#include "cfd/cfd_transaction.h"
#include "cfdcore/cfdcore_address.h"
#include "cfdcore/cfdcore_coin.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_iterator.h"
#include "cfdcore/cfdcore_key.h"
#include "cfdcore/cfdcore_logger.h"
#include "cfdcore/cfdcore_script.h"
#include "cfdcore/cfdcore_transaction.h"
#include "cfdcore/cfdcore_util.h"

#include "cfd/cfdapi_address.h"
#include "cfd/cfdapi_elements_transaction.h"
#include "cfd/cfdapi_struct.h"
#include "cfd/cfdapi_transaction.h"
#include "cfdapi_internal.h"  // NOLINT

namespace cfd {
namespace api {

#ifndef CFD_DISABLE_ELEMENTS
using cfd::api::ElementsTransactionApi;
#endif  // CFD_DISABLE_ELEMENTS
using cfd::AddressUtil;
using cfd::ScriptUtil;
using cfd::TransactionController;
using cfd::api::AddressApi;
using cfdcore::Address;
using cfdcore::AddressType;
using cfdcore::Amount;
using cfdcore::ByteData;
using cfdcore::ByteData160;
using cfdcore::CfdError;
using cfdcore::CfdException;
using cfdcore::CryptoUtil;
using cfdcore::IteratorWrapper;
using cfdcore::kByteData160Length;
using cfdcore::kByteData256Length;
using cfdcore::kScriptHashP2pkhLength;
using cfdcore::kScriptHashP2shLength;
using cfdcore::NetType;
using cfdcore::Pubkey;
using cfdcore::Script;
using cfdcore::ScriptBuilder;
using cfdcore::ScriptElement;
using cfdcore::ScriptElementType;
using cfdcore::ScriptOperator;
using cfdcore::ScriptType;
using cfdcore::ScriptWitness;
using cfdcore::SigHashAlgorithm;
using cfdcore::SigHashType;
using cfdcore::StringUtil;
using cfdcore::Transaction;
using cfdcore::Txid;
using cfdcore::TxIn;
using cfdcore::TxInReference;
using cfdcore::TxOutReference;
using cfdcore::WitnessVersion;
using cfdcore::logger::warn;

// -----------------------------------------------------------------------------
// ファイル内関数
// -----------------------------------------------------------------------------
/**
 * @brief AddMultisigSignのリクエスト情報チェックを行います。
 * @param[in] req       リクエスト情報
 * @param[in] addr_type アドレス種別
 */
static void ValidateAddMultisigSignRequest(  // linefeed
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

// -----------------------------------------------------------------------------
// TransactionApiクラス
// -----------------------------------------------------------------------------
CreateRawTransactionResponseStruct TransactionApi::CreateRawTransaction(
    const CreateRawTransactionRequestStruct& request) {
  auto call_func = [](const CreateRawTransactionRequestStruct& request)
      -> CreateRawTransactionResponseStruct {  // NOLINT
    CreateRawTransactionResponseStruct response;
    // validate version number
    const uint32_t tx_ver = request.version;
    if (4 < tx_ver) {
      warn(
          CFD_LOG_SOURCE,
          "Failed to CreateRawTransaction. invalid version number: version={}",  // NOLINT
          tx_ver);
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Invalid version number. We supports only 1, 2, 3, or 4:");
    }

    // TransactionController作成
    TransactionController txc(tx_ver, request.locktime);

    // TxInの追加
    {
      const uint32_t kDisableLockTimeSequence =
          TransactionController::GetLockTimeDisabledSequence();
      for (TxInRequestStruct txin_req : request.txins) {
        // TxInのunlocking_scriptは空で作成
        if (kDisableLockTimeSequence == txin_req.sequence) {
          txc.AddTxIn(
              Txid(txin_req.txid), txin_req.vout, txc.GetDefaultSequence());
        } else {
          txc.AddTxIn(Txid(txin_req.txid), txin_req.vout, txin_req.sequence);
        }
      }
    }

    // TxOutの追加
    {
      for (TxOutRequestStruct txout_req : request.txouts) {
        txc.AddTxOut(
            Address(txout_req.address),
            Amount::CreateBySatoshiAmount(txout_req.amount));
      }
    }

    response.hex = txc.GetHex();
    return response;
  };

  CreateRawTransactionResponseStruct result;
  result = ExecuteStructApi<
      CreateRawTransactionRequestStruct, CreateRawTransactionResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

DecodeRawTransactionResponseStruct TransactionApi::DecodeRawTransaction(
    const DecodeRawTransactionRequestStruct& request) {
  auto call_func = [](const DecodeRawTransactionRequestStruct& request)
      -> DecodeRawTransactionResponseStruct {  // NOLINT
    DecodeRawTransactionResponseStruct response;
    // validate version number
    const std::string& hex_string = request.hex;
    if (hex_string.empty()) {
      warn(
          CFD_LOG_SOURCE, "Failed to DecodeRawTransactionRequest. hex empty.");
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Invalid hex string. empty data.");
    }
    // TODO(k-matsuzawa): 引数のiswitness未使用。bitcoincoreの指定方法が不明瞭
    // // NOLINT

    NetType net_type = AddressApi::ConvertNetType(request.network);

    // TransactionController作成
    TransactionController txc(hex_string);
    const Transaction& tx = txc.GetTransaction();

    response.txid = tx.GetTxid().GetHex();
    // Decode時はTxidと同様にリバースで出力
    response.hash = Txid(tx.GetWitnessHash()).GetHex();
    response.size = tx.GetTotalSize();
    response.vsize = tx.GetVsize();
    response.weight = tx.GetWeight();
    response.version = tx.GetVersion();
    response.locktime = tx.GetLockTime();

    // TxInの追加
    for (const TxInReference& tx_in_ref : tx.GetTxInList()) {
      DecodeRawTransactionTxInStruct res_txin;
      res_txin.txid = tx_in_ref.GetTxid().GetHex();
      res_txin.vout = tx_in_ref.GetVout();
      if (!tx_in_ref.GetUnlockingScript().IsEmpty()) {
        res_txin.script_sig.asm_ = tx_in_ref.GetUnlockingScript().ToString();
        res_txin.script_sig.hex = tx_in_ref.GetUnlockingScript().GetHex();
      }
      for (const ByteData& witness :
           tx_in_ref.GetScriptWitness().GetWitness()) {  // NOLINT
        res_txin.txinwitness.push_back(witness.GetHex());
      }
      if (res_txin.txinwitness.empty()) {
        // txinwitnessを除外
        res_txin.ignore_items.insert("txinwitness");
      }
      res_txin.sequence = tx_in_ref.GetSequence();
      response.vin.push_back(res_txin);
    }

    // TxOutの追加
    int32_t txout_count = 0;
    for (const TxOutReference& txout_ref : tx.GetTxOutList()) {
      DecodeRawTransactionTxOutStruct res_txout;
      res_txout.value = txout_ref.GetValue().GetCoinValue();
      res_txout.n = txout_count;

      std::vector<std::string> addresses;
      Script locking_script = txout_ref.GetLockingScript();
      std::vector<ScriptElement> script_element =
          locking_script.GetElementList();
      res_txout.script_pub_key.hex = locking_script.GetHex();
      res_txout.script_pub_key.asm_ = locking_script.ToString();

      if (locking_script.IsEmpty()) {
        res_txout.script_pub_key.type = "nonstandard";
        res_txout.script_pub_key.ignore_items.insert("reqSigs");
        res_txout.script_pub_key.ignore_items.insert("addresses");
      } else {
        res_txout.script_pub_key.type = "nonstandard";

        bool is_witness = false;
        int top_number = static_cast<int>(script_element[0].GetNumber());

        ScriptElement last_element = script_element.back();

        // 現状、WitnessVersion0のみ
        if ((script_element.size() == 2) && script_element[0].IsNumber() &&
            script_element[1].IsBinary() &&
            ((top_number >= 0) && (top_number <= 16))) {
          size_t buffer_array_size =
              script_element[1].GetBinaryData().GetDataSize();
          if ((buffer_array_size == kByteData160Length) ||
              (buffer_array_size == kByteData256Length)) {
            // P2WPKH or P2WSH
            is_witness = true;

            if (top_number == 0) {
              if (buffer_array_size == kByteData160Length) {
                // P2WPKH
                res_txout.script_pub_key.type = "witness_v0_keyhash";
              } else if (buffer_array_size == kByteData256Length) {
                // P2WSH
                res_txout.script_pub_key.type = "witness_v0_scripthash";
              }
            } else {
              // unsupported target witness version.
              res_txout.script_pub_key.type = "witness_unknown";
            }
          }
        }

        if (is_witness) {
          res_txout.script_pub_key.req_sigs = 1;
          Address addr(
              net_type, WitnessVersion::kVersion0,
              script_element[1].GetBinaryData());
          res_txout.script_pub_key.addresses.push_back(addr.GetAddress());
        } else if (CheckMultiSigScript(locking_script)) {
          // MultiSig
          int64_t sig_num = top_number;
          res_txout.script_pub_key.req_sigs = sig_num;
          res_txout.script_pub_key.type = "multisig";
          for (size_t index = 1; index < script_element.size() - 2;
               ++index) {  // NOLINT
            Address addr(
                net_type, Pubkey(script_element[index].GetBinaryData()));
            res_txout.script_pub_key.addresses.push_back(addr.GetAddress());
          }
        } else if (CheckP2pkhScript(locking_script)) {
          res_txout.script_pub_key.req_sigs = 1;
          res_txout.script_pub_key.type = "pubkeyhash";
          Address addr(
              net_type, AddressType::kP2pkhAddress,
              ByteData160(script_element[2].GetBinaryData().GetBytes()));
          res_txout.script_pub_key.addresses.push_back(addr.GetAddress());
        } else if (CheckP2shScript(locking_script)) {
          res_txout.script_pub_key.req_sigs = 1;
          res_txout.script_pub_key.type = "scripthash";
          Address addr(
              net_type, AddressType::kP2shAddress,
              ByteData160(script_element[1].GetBinaryData().GetBytes()));
          res_txout.script_pub_key.addresses.push_back(addr.GetAddress());
        } else if (CheckPubkeyScript(locking_script)) {
          res_txout.script_pub_key.req_sigs = 1;
          res_txout.script_pub_key.type = "pubkey";
          Address addr(net_type, Pubkey(script_element[0].GetBinaryData()));
          res_txout.script_pub_key.addresses.push_back(addr.GetAddress());
        } else if (CheckNullDataScript(locking_script)) {
          res_txout.script_pub_key.type = "nulldata";
          res_txout.script_pub_key.ignore_items.insert("reqSigs");
          res_txout.script_pub_key.ignore_items.insert("addresses");
        } else {
          // nonstandard or witness_unknown
          res_txout.script_pub_key.ignore_items.insert("reqSigs");
          res_txout.script_pub_key.ignore_items.insert("addresses");
        }
      }

      response.vout.push_back(res_txout);
      ++txout_count;
    }
    return response;
  };

  DecodeRawTransactionResponseStruct result;
  result = ExecuteStructApi<
      DecodeRawTransactionRequestStruct, DecodeRawTransactionResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

GetWitnessStackNumResponseStruct TransactionApi::GetWitnessStackNum(
    const GetWitnessStackNumRequestStruct& request) {
  auto call_func = [](const GetWitnessStackNumRequestStruct& request)
      -> GetWitnessStackNumResponseStruct {  // NOLINT
    GetWitnessStackNumResponseStruct response;
    std::string hex_string = request.tx_hex;
    if (hex_string.empty()) {
      warn(CFD_LOG_SOURCE, "Failed to GetWitnessStackNum. hex empty.");
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Invalid hex string. empty data.");
    }

    // TransactionController作成
    TransactionController txc(hex_string);

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

AddSignResponseStruct TransactionApi::AddSign(
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
    TransactionController txc(hex_string);

    if (request.is_witness) {
      // Witnessの追加
      Txid txid = Txid(request.txin_txid);
      if (request.clear_stack) {
        txc.RemoveWitnessStackAll(txid, request.txin_vout);
      }
      std::vector<ByteData> witness_datas;
      for (const SignDataStruct& stack_req : request.sign_param) {
        ByteData byte_data = ConvertSignDataToSignature(
            stack_req.hex, (stack_req.type == "sign"), stack_req.der_encode,
            stack_req.sighash_type, stack_req.sighash_anyone_can_pay);
        witness_datas.push_back(byte_data);
      }
      txc.AddWitnessStack(txid, request.txin_vout, witness_datas);
    } else {
      std::vector<ByteData> unlock_script;
      for (const SignDataStruct& stack_req : request.sign_param) {
        ByteData byte_data = ConvertSignDataToSignature(
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

UpdateWitnessStackResponseStruct TransactionApi::UpdateWitnessStack(
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
    TransactionController txc(hex_string);

    // Witnessの更新
    const WitnessStackDataStruct& stack_req = request.witness_stack;
    ByteData byte_data = ConvertSignDataToSignature(
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

const std::vector<Pubkey> TransactionApi::ExtractPubkeysFromMultisigScript(
    const Script& multisig_script) {
  std::vector<Pubkey> pubkeys;

  const std::vector<ScriptElement> elements = multisig_script.GetElementList();

  // find OP_CHECKMULTISIG or OP_CHECKMULTISIGVERIFY
  IteratorWrapper<ScriptElement> itr = IteratorWrapper<ScriptElement>(
      elements, "Invalid script element access", true);
  // search OP_CHECKMULTISIG(or VERIFY)
  while (itr.hasNext()) {
    ScriptElement element = itr.next();
    if (!element.IsOpCode()) {
      continue;
    }
    if (element.GetOpCode() == ScriptOperator::OP_CHECKMULTISIG ||
        element.GetOpCode() == ScriptOperator::OP_CHECKMULTISIGVERIFY) {
      break;
    }
  }
  // target opcode not found
  if (!itr.hasNext()) {
    warn(
        CFD_LOG_SOURCE,
        "Multisig opcode (OP_CHECKMULTISIG|VERIFY) not found"
        " in redeem script: script={}",
        multisig_script.ToString());
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "OP_CHCKMULTISIG(OP_CHECKMULTISIGVERIFY) not found"
        " in redeem script.");
  }

  // get contain pubkey num
  const ScriptElement& op_m = itr.next();
  if (!op_m.IsNumber()) {
    warn(
        CFD_LOG_SOURCE,
        "Invalid OP_CHECKMULTISIG(VERIFY) input in redeem script."
        " Missing contain pubkey number.: script={}",
        multisig_script.ToString());
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Invalid OP_CHCKMULTISIG(OP_CHECKMULTISIGVERIFY) input"
        " in redeem script. Missing contain pubkey number.");
  }

  // set pubkey to vector(reverse data)
  int64_t contain_pubkey_num = op_m.GetNumber();
  for (int64_t i = 0; i < contain_pubkey_num; ++i) {
    if (!itr.hasNext()) {
      warn(
          CFD_LOG_SOURCE,
          "Not found enough pubkeys in redeem script.: "
          "require_pubkey_num={}, script={}",
          contain_pubkey_num, multisig_script.ToString());
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Not found enough pubkeys in redeem script.");
    }

    const ScriptElement& pubkey_element = itr.next();
    // check script element type
    if (!pubkey_element.IsBinary()) {
      warn(
          CFD_LOG_SOURCE,
          "Invalid script element. Not binary element.: "
          "ScriptElementType={}, data={}",
          pubkey_element.GetType(), pubkey_element.ToString());
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Invalid ScriptElementType.(not binary)");
    }

    // push pubkey data
    pubkeys.push_back(Pubkey(pubkey_element.GetBinaryData()));
  }

  // check opcode(require signature num)
  if (!itr.hasNext() || !itr.next().IsNumber()) {
    warn(
        CFD_LOG_SOURCE,
        "Invalid OP_CHECKMULTISIG(VERIFY) input in redeem script."
        " Missing require signature number.: script={}",
        multisig_script.ToString());
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Invalid OP_CHCKMULTISIG(OP_CHECKMULTISIGVERIFY) input"
        " in redeem script. Missing require signature number.");
  }

  // return reverse pubkey vector
  std::reverse(std::begin(pubkeys), std::end(pubkeys));
  return pubkeys;
}

AddMultisigSignResponseStruct TransactionApi::AddMultisigSign(
    const AddMultisigSignRequestStruct& request) {
  auto call_func = [](const AddMultisigSignRequestStruct& request)
      -> AddMultisigSignResponseStruct {  // NOLINT
    AddMultisigSignResponseStruct response;
    // validate request
    AddressType addr_type = AddressApi::ConvertAddressType(request.txin_type);
    ValidateAddMultisigSignRequest(request, addr_type);

    const std::string& hex_string = request.tx_hex;
    TransactionController txc(hex_string);

    // extract pubkeys from redeem script
    // ValidateAddMultiSignRequest ensures that we have one of three correct
    // types.
    Script redeem_script = addr_type == AddressType::kP2shAddress
                               ? Script(request.redeem_script)
                               : Script(request.witness_script);

    std::vector<Pubkey> pubkeys =
        ExtractPubkeysFromMultisigScript(redeem_script);
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
        // unreachable, valid type checked by validate method.
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

SigHashType TransactionApi::ConvertSigHashType(
    const std::string& sighash_type_string, bool is_anyone_can_pay) {
  std::string check_string = sighash_type_string;
  std::transform(
      check_string.begin(), check_string.end(), check_string.begin(),
      ::tolower);
  if (check_string == "all") {
    return SigHashType(SigHashAlgorithm::kSigHashAll, is_anyone_can_pay);
  } else if (check_string == "none") {
    return SigHashType(SigHashAlgorithm::kSigHashNone, is_anyone_can_pay);
  } else if (check_string == "single") {
    return SigHashType(SigHashAlgorithm::kSigHashSingle, is_anyone_can_pay);
  }
  warn(
      CFD_LOG_SOURCE,
      "Failed to CreateMultisig. Invalid sighash_type: sighashType={}",
      sighash_type_string);
  throw CfdException(
      CfdError::kCfdIllegalArgumentError,
      "Invalid sighashType. sighashType must be "
      "\"all, none, single\".");
}

bool TransactionApi::CheckMultiSigScript(const Script& script) {
  bool is_match = false;
  std::vector<ScriptElement> script_element = script.GetElementList();

  if (script_element.size() < 4) {
    // 不足
  } else {
    for (size_t index = 0; index < script_element.size(); ++index) {
      if ((index == 0) || (index == (script_element.size() - 2))) {
        int value = static_cast<int>(script_element[index].GetNumber());
        if ((value >= 1) && (value <= 16)) {
          // OK (1～16)
        } else {
          break;
        }
      } else if (index == (script_element.size() - 1)) {
        ScriptOperator op_code = script_element[index].GetOpCode();

        if (op_code == ScriptOperator::OP_CHECKMULTISIG) {
          // CheckMultiSigVerifyは関係ない
          is_match = true;
        }
        // 終端なのでbreakは行わない
      } else {
        size_t data_size = script_element[index].GetBinaryData().GetDataSize();
        if (script_element[index].IsBinary() &&
            ((data_size == Pubkey::kCompressedPubkeySize) ||
             (data_size == Pubkey::kPubkeySize))) {
          // Pubkey
        } else {
          break;
        }
      }
    }
  }
  return is_match;
}

bool TransactionApi::CheckP2pkhScript(const Script& script) {
  // OP_DUP OP_HASH160 [HASH160] OP_EQUALVERIFY OP_CHECKSIG
  bool is_match = false;
  std::vector<ScriptElement> script_element = script.GetElementList();
  uint32_t length = static_cast<uint32_t>(script.GetData().GetDataSize());

  if (script_element.size() != 5) {
    // unmatch count
  } else if (length != kScriptHashP2pkhLength) {
    // unmatch count
  } else if (script_element[0].GetOpCode() != ScriptOperator::OP_DUP) {
    // unmatch opcode
  } else if (script_element[1].GetOpCode() != ScriptOperator::OP_HASH160) {
    // unmatch opcode
  } else if (script_element[3].GetOpCode() != ScriptOperator::OP_EQUALVERIFY) {
    // unmatch opcode
  } else if (script_element[4].GetOpCode() != ScriptOperator::OP_CHECKSIG) {
    // unmatch opcode
  } else if (!script_element[2].IsBinary()) {
    // unmatch type
  } else {
    // サイズは比較済み
    is_match = true;
  }
  return is_match;
}

bool TransactionApi::CheckP2shScript(const Script& script) {
  // OP_HASH160 [HASH160] OP_EQUAL
  bool is_match = false;
  std::vector<ScriptElement> script_element = script.GetElementList();
  uint32_t length = static_cast<uint32_t>(script.GetData().GetDataSize());

  if (script_element.size() != 3) {
    // unmatch count
  } else if (length != kScriptHashP2shLength) {
    // unmatch count
  } else if (script_element[0].GetOpCode() != ScriptOperator::OP_HASH160) {
    // unmatch opcode
  } else if (script_element[2].GetOpCode() != ScriptOperator::OP_EQUAL) {
    // unmatch opcode
  } else if (!script_element[1].IsBinary()) {
    // unmatch type
  } else {
    // サイズは比較済み
    is_match = true;
  }
  return is_match;
}

bool TransactionApi::CheckPubkeyScript(const Script& script) {
  // <pubkey> OP_CHECKSIG
  bool is_match = false;
  std::vector<ScriptElement> script_element = script.GetElementList();

  if (script_element.size() != 2) {
    // unmatch count
  } else if (script_element[1].GetOpCode() != ScriptOperator::OP_CHECKSIG) {
    // unmatch opcode
  } else {
    size_t data_size = script_element[0].GetBinaryData().GetDataSize();
    if (script_element[0].IsBinary() &&
        ((data_size == Pubkey::kCompressedPubkeySize) ||
         (data_size == Pubkey::kPubkeySize))) {
      // Pubkey
      is_match = true;
    }
  }
  return is_match;
}

bool TransactionApi::CheckNullDataScript(const Script& script) {
  // OP_RETURN <0 to 40 bytes of data>
  static constexpr uint32_t kNullDataMaxSize = 40 + 1 + 1;
  bool is_match = false;
  std::vector<ScriptElement> script_element = script.GetElementList();
  uint32_t length = static_cast<uint32_t>(script.GetData().GetDataSize());

  if (script_element.size() == 0) {
    // unmatch count
  } else if (length > kNullDataMaxSize) {
    // unmatch length
  } else if (script_element[0].GetOpCode() != ScriptOperator::OP_RETURN) {
    // unmatch opcode
  } else if (script_element.size() == 1) {
    // op_return only.
    is_match = true;
  } else {
    uint32_t count = 0;
    for (size_t index = 1; index < script_element.size(); ++index) {
      if (script_element[index].IsNumber() ||
          script_element[index].IsBinary()) {
        ++count;
      }
    }
    if (static_cast<uint32_t>(script_element.size()) == (count + 1)) {
      is_match = true;
    }
  }
  return is_match;
}

ByteData TransactionApi::ConvertSignDataToSignature(
    const std::string& hex_string, bool is_sign, bool is_der_encode,
    const std::string& sighash_type, bool sighash_anyone_can_pay) {
  ByteData byte_data;
  if (is_sign && is_der_encode) {
    if (hex_string.empty()) {
      warn(CFD_LOG_SOURCE, "Failed to AddMultisigSign. sign hex empty.");
      throw CfdException(
          CfdError::kCfdIllegalArgumentError,
          "Invalid hex string. empty sign hex.");
    }
    SigHashType sighashtype = TransactionApi::ConvertSigHashType(
        sighash_type, sighash_anyone_can_pay);
    byte_data = CryptoUtil::ConvertSignatureToDer(hex_string, sighashtype);
  } else if (hex_string.empty()) {
    // do nothing
  } else {
    byte_data = ByteData(hex_string);
  }
  return byte_data;
}

}  // namespace api
}  // namespace cfd
