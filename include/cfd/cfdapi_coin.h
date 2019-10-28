// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_coin.h
 *
 * @brief cfd-apiで利用するCoin関連のクラス定義
 */
#ifndef CFD_INCLUDE_CFD_CFDAPI_COIN_H_
#define CFD_INCLUDE_CFD_CFDAPI_COIN_H_

#include <string>
#include <vector>

#include "cfd/cfd_common.h"
#include "cfd/cfd_transaction.h"
#include "cfd/cfd_utxo.h"
#include "cfdcore/cfdcore_bytedata.h"
#include "cfdcore/cfdcore_script.h"
#include "cfdcore/cfdcore_util.h"

namespace cfd {
namespace api {

using cfd::TransactionController;
using cfd::core::AddressType;
using cfd::core::Amount;
using cfd::core::ByteData;
using cfd::core::HashType;
using cfd::core::Pubkey;
using cfd::core::Script;
using cfd::core::SigHashType;
using cfd::core::Txid;

/**
 * @brief Coin関連のAPIクラス
 */
class CFD_EXPORT CoinApi {
 public:
  /**
   * @brief constructor
   */
  CoinApi() {}

};

}  // namespace api
}  // namespace cfd

#endif  // CFD_INCLUDE_CFD_CFDAPI_COINSELECTION_H_
