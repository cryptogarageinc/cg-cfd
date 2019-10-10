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
#include "cfdapi_internal.h"          // NOLINT
#include "cfdapi_transaction_base.h"  // NOLINT

namespace cfd {
namespace api {

using cfd::core::CfdError;
using cfd::core::CfdException;
using cfd::core::Txid;
using cfd::core::logger::warn;

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

TransactionController TransactionApi::CreateRawTransaction(
    uint32_t version, uint32_t locktime, const std::vector<TxIn>& txins,
    const std::vector<TxOut>& txouts) const {
  if (4 < version) {
    warn(
        CFD_LOG_SOURCE,
        "Failed to CreateRawTransaction. invalid version number: version={}",  // NOLINT
        version);
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Invalid version number. We supports only 1, 2, 3, or 4:");
  }

  // TransactionController作成
  TransactionController txc(version, locktime);

  // TxInの追加
  const uint32_t kDisableLockTimeSequence =
      TransactionController::GetLockTimeDisabledSequence();
  for (TxIn txin : txins) {
    // TxInのunlocking_scriptは空で作成
    if (kDisableLockTimeSequence == txin.GetSequence()) {
      txc.AddTxIn(txin.GetTxid(), txin.GetVout(), txc.GetDefaultSequence());
    } else {
      txc.AddTxIn(txin.GetTxid(), txin.GetVout(), txin.GetSequence());
    }
  }

  // TxOutの追加
  for (TxOut txout : txouts) {
    txc.AddTxOut(txout.GetLockingScript(), txout.GetValue());
  }

  return txc;
}

ByteData TransactionApi::CreateSignatureHash(
    const std::string& tx_hex, const TxInReference& txin, const Pubkey& pubkey,
    const Amount& amount, HashType hash_type,
    const SigHashType& sighash_type) const {
  return CreateSignatureHash(
      tx_hex, txin, pubkey.GetData(), amount, hash_type, sighash_type);
}

ByteData TransactionApi::CreateSignatureHash(
    const std::string& tx_hex, const TxInReference& txin,
    const Script& redeem_script, const Amount& amount, HashType hash_type,
    const SigHashType& sighash_type) const {
  return CreateSignatureHash(
      tx_hex, txin, redeem_script.GetData(), amount, hash_type, sighash_type);
}

ByteData TransactionApi::CreateSignatureHash(
    const std::string& tx_hex, const TxInReference& txin,
    const ByteData& key_data, const Amount& amount, HashType hash_type,
    const SigHashType& sighash_type) const {
  std::string sig_hash;
  int64_t amount_value = amount.GetSatoshiValue();
  const Txid& txid = txin.GetTxid();
  uint32_t vout = txin.GetVout();
  TransactionController txc(tx_hex);

  if (hash_type == HashType::kP2pkh) {
    sig_hash = txc.CreateP2pkhSignatureHash(
        txid, vout,  // vout
        Pubkey(key_data), sighash_type);
  } else if (hash_type == HashType::kP2sh) {
    sig_hash = txc.CreateP2shSignatureHash(
        txid, vout, Script(key_data), sighash_type);
  } else if (hash_type == HashType::kP2wpkh) {
    sig_hash = txc.CreateP2wpkhSignatureHash(
        txid, vout, Pubkey(key_data), sighash_type,
        Amount::CreateBySatoshiAmount(amount_value));
  } else if (hash_type == HashType::kP2wsh) {
    sig_hash = txc.CreateP2wshSignatureHash(
        txid, vout, Script(key_data), sighash_type,
        Amount::CreateBySatoshiAmount(amount_value));
  } else {
    warn(
        CFD_LOG_SOURCE,
        "Failed to CreateSignatureHash. Invalid hash_type:  "
        "hash_type={}",  // NOLINT
        hash_type);
    throw CfdException(
        CfdError::kCfdIllegalArgumentError,
        "Invalid hash_type. hash_type must be \"p2pkh\"(0) "
        "or \"p2sh\"(1) or \"p2wpkh\"(2) or \"p2wsh\"(3).");  // NOLINT
  }

  return ByteData(sig_hash);
}

TransactionController TransactionApi::AddMultisigSign(
    const std::string& tx_hex, const TxInReference& txin,
    const std::vector<SignParameter>& sign_list, AddressType address_type,
    const Script& witness_script, const Script redeem_script,
    bool clear_stack) {
  std::string result =
      TransactionApiBase::AddMultisigSign<TransactionController>(
          tx_hex, txin, sign_list, address_type, witness_script, redeem_script,
          clear_stack, CreateController);
  return TransactionController(result);
}

}  // namespace api
}  // namespace cfd

namespace cfd {
namespace js {
namespace api {

using cfd::TransactionController;
using cfd::api::TransactionApi;
using cfd::core::Address;
using cfd::core::AddressType;
using cfd::core::Amount;
using cfd::core::ByteData;
using cfd::core::ByteData160;
using cfd::core::CfdError;
using cfd::core::CfdException;
using cfd::core::CryptoUtil;
using cfd::core::HashType;
using cfd::core::kByteData160Length;
using cfd::core::kByteData256Length;
using cfd::core::kScriptHashP2pkhLength;
using cfd::core::kScriptHashP2shLength;
using cfd::core::NetType;
using cfd::core::Pubkey;
using cfd::core::Script;
using cfd::core::ScriptElement;
using cfd::core::ScriptOperator;
using cfd::core::SigHashType;
using cfd::core::Transaction;
using cfd::core::Txid;
using cfd::core::TxIn;
using cfd::core::TxInReference;
using cfd::core::TxOut;
using cfd::core::TxOutReference;
using cfd::core::WitnessVersion;
using cfd::core::logger::warn;
using cfd::js::api::AddressStructApi;

// -----------------------------------------------------------------------------
// TransactionStructApiクラス
// -----------------------------------------------------------------------------
CreateRawTransactionResponseStruct TransactionStructApi::CreateRawTransaction(
    const CreateRawTransactionRequestStruct& request) {
  auto call_func = [](const CreateRawTransactionRequestStruct& request)
      -> CreateRawTransactionResponseStruct {  // NOLINT
    CreateRawTransactionResponseStruct response;

    std::vector<TxIn> txins;
    for (TxInRequestStruct txin_req : request.txins) {
      // TxInのunlocking_scriptは空で作成
      TxIn txin(Txid(txin_req.txid), txin_req.vout, txin_req.sequence);
      txins.push_back(txin);
    }

    std::vector<TxOut> txouts;
    for (TxOutRequestStruct txout_req : request.txouts) {
      Amount amount = Amount::CreateBySatoshiAmount(txout_req.amount);
      Address address(txout_req.address);
      TxOut txout(amount, address);
      txouts.push_back(txout);
    }

    TransactionApi api;
    TransactionController txc = api.CreateRawTransaction(
        request.version, request.locktime, txins, txouts);

    response.hex = txc.GetHex();
    return response;
  };

  CreateRawTransactionResponseStruct result;
  result = ExecuteStructApi<
      CreateRawTransactionRequestStruct, CreateRawTransactionResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

DecodeRawTransactionResponseStruct TransactionStructApi::DecodeRawTransaction(
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

    NetType net_type = AddressStructApi::ConvertNetType(request.network);

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
      if (tx.IsCoinBase()) {
        res_txin.ignore_items.insert("txid");
        res_txin.ignore_items.insert("vout");
        res_txin.ignore_items.insert("scriptSig");
        res_txin.ignore_items.insert("txinwitness");

        if (!tx_in_ref.GetUnlockingScript().IsEmpty()) {
          res_txin.coinbase = tx_in_ref.GetUnlockingScript().GetHex();
        }
      } else {
        res_txin.ignore_items.insert("coinbase");

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

GetWitnessStackNumResponseStruct TransactionStructApi::GetWitnessStackNum(
    const GetWitnessStackNumRequestStruct& request) {
  auto call_func = [](const GetWitnessStackNumRequestStruct& request)
      -> GetWitnessStackNumResponseStruct {  // NOLINT
    return TransactionStructApiBase::GetWitnessStackNum<TransactionController>(
        request, cfd::api::CreateController);
  };

  GetWitnessStackNumResponseStruct result;
  result = ExecuteStructApi<
      GetWitnessStackNumRequestStruct, GetWitnessStackNumResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

AddSignResponseStruct TransactionStructApi::AddSign(
    const AddSignRequestStruct& request) {
  auto call_func =
      [](const AddSignRequestStruct& request) -> AddSignResponseStruct {
    return TransactionStructApiBase::AddSign<TransactionController>(
        request, cfd::api::CreateController);
  };

  AddSignResponseStruct result;
  result = ExecuteStructApi<AddSignRequestStruct, AddSignResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

UpdateWitnessStackResponseStruct TransactionStructApi::UpdateWitnessStack(
    const UpdateWitnessStackRequestStruct& request) {
  auto call_func = [](const UpdateWitnessStackRequestStruct& request)
      -> UpdateWitnessStackResponseStruct {  // NOLINT
    return TransactionStructApiBase::UpdateWitnessStack<TransactionController>(
        request, cfd::api::CreateController);
  };

  UpdateWitnessStackResponseStruct result;
  result = ExecuteStructApi<
      UpdateWitnessStackRequestStruct, UpdateWitnessStackResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

AddMultisigSignResponseStruct TransactionStructApi::AddMultisigSign(
    const AddMultisigSignRequestStruct& request) {
  auto call_func = [](const AddMultisigSignRequestStruct& request)
      -> AddMultisigSignResponseStruct {  // NOLINT
    TxInReference txin(TxIn(Txid(request.txin.txid), request.txin.vout, 0));
    AddressType addr_type =
        AddressStructApi::ConvertAddressType(request.txin.hash_type);
    Script redeem_script(request.txin.redeem_script);
    Script witness_script(request.txin.witness_script);
    std::vector<SignParameter> sign_list;

    SignParameter sign_data;
    for (const auto& stack_req : request.txin.sign_params) {
      sign_data =
          TransactionStructApiBase::ConvertSignDataStructToSignParameter(
              stack_req);
      if (!stack_req.related_pubkey.empty()) {
        sign_data.SetRelatedPubkey(Pubkey(stack_req.related_pubkey));
      }
      sign_list.push_back(sign_data);
    }

    TransactionApi api;
    TransactionController ctx = api.AddMultisigSign(
        request.tx, txin, sign_list, addr_type, witness_script, redeem_script,
        request.txin.clear_stack);

    AddMultisigSignResponseStruct response;
    response.hex = ctx.GetHex();
    return response;
  };

  AddMultisigSignResponseStruct result;
  result = ExecuteStructApi<
      AddMultisigSignRequestStruct, AddMultisigSignResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

CreateSignatureHashResponseStruct TransactionStructApi::CreateSignatureHash(
    const CreateSignatureHashRequestStruct& request) {
  auto call_func = [](const CreateSignatureHashRequestStruct& request)
      -> CreateSignatureHashResponseStruct {  // NOLINT
    CreateSignatureHashResponseStruct response;

    ByteData sig_hash;
    Amount amount = Amount::CreateBySatoshiAmount(request.txin.amount);
    const Txid& txid = Txid(request.txin.txid);
    uint32_t vout = request.txin.vout;
    const std::string& hashtype_str = request.txin.hash_type;
    SigHashType sighashtype = TransactionStructApiBase::ConvertSigHashType(
        request.txin.sighash_type, request.txin.sighash_anyone_can_pay);

    Pubkey pubkey;
    Script script;
    if (request.txin.key_data.type == "pubkey") {
      pubkey = Pubkey(request.txin.key_data.hex);
    } else if (request.txin.key_data.type == "redeem_script") {
      script = Script(request.txin.key_data.hex);
    }

    TransactionApi api;
    HashType hash_type;
    TxInReference txin_ref(TxIn(txid, vout, 0));
    if (hashtype_str == "p2pkh" || hashtype_str == "p2wpkh") {
      hash_type =
          (hashtype_str == "p2pkh") ? HashType::kP2pkh : HashType::kP2wpkh;
      sig_hash = api.CreateSignatureHash(
          request.tx, txin_ref, pubkey, amount, hash_type, sighashtype);

    } else if (hashtype_str == "p2sh" || hashtype_str == "p2wsh") {
      hash_type =
          (hashtype_str == "p2sh") ? HashType::kP2sh : HashType::kP2wsh;
      sig_hash = api.CreateSignatureHash(
          request.tx, txin_ref, script, amount, hash_type, sighashtype);

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

    // レスポンスとなるモデルへ変換
    response.sighash = sig_hash.GetHex();
    return response;
  };

  CreateSignatureHashResponseStruct result;
  result = ExecuteStructApi<
      CreateSignatureHashRequestStruct, CreateSignatureHashResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

bool TransactionStructApi::CheckMultiSigScript(const Script& script) {
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

bool TransactionStructApi::CheckP2pkhScript(const Script& script) {
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

bool TransactionStructApi::CheckP2shScript(const Script& script) {
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

bool TransactionStructApi::CheckPubkeyScript(const Script& script) {
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

bool TransactionStructApi::CheckNullDataScript(const Script& script) {
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
}  // namespace js
}  // namespace cfd
