// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_sighash.cpp
 *
 * @brief cfd-apiで利用するSigHash生成の実装ファイル
 */
#include <string>

#include "cfd/cfd_transaction.h"
#include "cfdcore/cfdcore_amount.h"
#include "cfdcore/cfdcore_exception.h"
#include "cfdcore/cfdcore_key.h"
#include "cfdcore/cfdcore_logger.h"
#include "cfdcore/cfdcore_script.h"
#include "cfdcore/cfdcore_transaction.h"
#include "cfdcore/cfdcore_util.h"

#include "cfd/cfdapi_sighash.h"
#include "cfd/cfdapi_transaction_base.h"
#include "cfdapi_internal.h"  // NOLINT

namespace cfd {
namespace api {

using cfd::TransactionController;
using cfdcore::Amount;
using cfdcore::CfdError;
using cfdcore::CfdException;
using cfdcore::Pubkey;
using cfdcore::Script;
using cfdcore::SigHashType;
using cfdcore::Transaction;
using cfdcore::Txid;
using cfdcore::logger::warn;

CreateSignatureHashResponseStruct SigHashApi::CreateSignatureHash(
    const CreateSignatureHashRequestStruct& request) {
  auto call_func = [](const CreateSignatureHashRequestStruct& request)
      -> CreateSignatureHashResponseStruct {  // NOLINT
    CreateSignatureHashResponseStruct response;
    std::string sig_hash;
    int64_t amount = request.amount;
    const std::string& hashtype_str = request.hash_type;
    const std::string& pubkey_hex = request.pubkey_hex;
    const std::string& script_hex = request.script_hex;
    const Txid& txid = Txid(request.txin_txid);
    uint32_t vout = request.txin_vout;
    TransactionController txc(request.tx_hex);
    SigHashType sighashtype = TransactionApiBase::ConvertSigHashType(
        request.sighash_type, request.sighash_anyone_can_pay);

    if (hashtype_str == "p2pkh") {
      sig_hash = txc.CreateP2pkhSignatureHash(
          txid, vout,  // vout
          Pubkey(pubkey_hex), sighashtype);
    } else if (hashtype_str == "p2sh") {
      sig_hash = txc.CreateP2shSignatureHash(
          txid, vout, Script(script_hex), sighashtype);
    } else if (hashtype_str == "p2wpkh") {
      sig_hash = txc.CreateP2wpkhSignatureHash(
          txid, vout, Pubkey(pubkey_hex), sighashtype,
          Amount::CreateBySatoshiAmount(amount));
    } else if (hashtype_str == "p2wsh") {
      sig_hash = txc.CreateP2wshSignatureHash(
          txid, vout, Script(script_hex), sighashtype,
          Amount::CreateBySatoshiAmount(amount));
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
    response.sighash = sig_hash;
    return response;
  };

  CreateSignatureHashResponseStruct result;
  result = ExecuteStructApi<
      CreateSignatureHashRequestStruct, CreateSignatureHashResponseStruct>(
      request, call_func, std::string(__FUNCTION__));
  return result;
}

}  // namespace api
}  // namespace cfd
