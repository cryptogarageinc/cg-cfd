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
#include "cfd/cfdapi_transaction_base.h"
#include "cfdapi_internal.h"  // NOLINT

namespace cfd {
namespace api {

#ifndef CFD_DISABLE_ELEMENTS
using cfd::api::ElementsTransactionApi;
#endif  // CFD_DISABLE_ELEMENTS
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
 * @brief Create a TransactionController object.
 *
 * @param[in] hex of the transaction for which to create the controller object
 * @return a TransactionController instance
 */
static TransactionController CreateController(const std::string& hex) {
  return TransactionController(hex);
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
      res_txout.value = txout_ref.GetValue().GetSatoshiValue();
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
    return TransactionApiBase::GetWitnessStackNum<TransactionController>(
        request, CreateController);
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
    return TransactionApiBase::AddSign<TransactionController>(
        request, CreateController);
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
    return TransactionApiBase::UpdateWitnessStack<TransactionController>(
        request, CreateController);
  };

  UpdateWitnessStackResponseStruct result;
  result = ExecuteStructApi<
      UpdateWitnessStackRequestStruct, UpdateWitnessStackResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

AddMultisigSignResponseStruct TransactionApi::AddMultisigSign(
    const AddMultisigSignRequestStruct& request) {
  auto call_func = [](const AddMultisigSignRequestStruct& request)
      -> AddMultisigSignResponseStruct {  // NOLINT
    return TransactionApiBase::AddMultisigSign<TransactionController>(
        request, CreateController);
  };

  AddMultisigSignResponseStruct result;
  result = ExecuteStructApi<
      AddMultisigSignRequestStruct, AddMultisigSignResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
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

}  // namespace api
}  // namespace cfd
