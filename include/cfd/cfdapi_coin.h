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
#include "cfdcore/cfdcore_elements_transaction.h"
#include "cfdcore/cfdcore_script.h"
#include "cfdcore/cfdcore_util.h"

namespace cfd {
namespace api {

using cfd::core::AddressType;
using cfd::core::Amount;
using cfd::core::BlockHash;
using cfd::core::Script;
using cfd::core::Txid;
#ifndef CFD_DISABLE_ELEMENTS
using cfd::core::ConfidentialAssetId;
#endif  // CFD_DISABLE_ELEMENTS

/**
 * @brief UTXO構造体
 */
struct UtxoData {
  uint64_t block_height;   //!< blick高
  BlockHash block_hash;    //!< block hash
  Script locking_script;   //!< script
  Address address;         //!< address
  std::string descriptor;  //!< output descriptor
  Amount amount;           //!< amount
#ifndef CFD_DISABLE_ELEMENTS
  ConfidentialAssetId asset;  //!< asset
#endif                        // CFD_DISABLE_ELEMENTS
#if 0
  int32_t status;           //!< utxo status (reserved)
  // elements
  ConfidentialAddress confidential_address;   //!< Confidential address
  BlindFactor asset_blind_factor;     //!< asset blind factor
  BlindFactor amount_blind_factor;    //!< blind vactor
#endif  // if 0
};

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

#endif  // CFD_INCLUDE_CFD_CFDAPI_COIN_H_
