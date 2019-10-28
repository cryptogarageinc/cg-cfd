// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_coinselection.h
 *
 * @brief cfd-apiで利用するCoinSelection関連のクラス定義
 */
#ifndef CFD_INCLUDE_CFD_CFDAPI_COINSELECTION_H_
#define CFD_INCLUDE_CFD_CFDAPI_COINSELECTION_H_

#include <string>
#include <vector>

#include "cfd/cfd_common.h"
#include "cfd/cfd_transaction.h"
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
 * @brief Transaction関連のAPIクラス
 */
class CFD_EXPORT CoinSelectionApi {
 public:
  /**
   * @brief constructor
   */
  CoinSelectionApi() {}

};

}  // namespace api
}  // namespace cfd

#endif  // CFD_INCLUDE_CFD_CFDAPI_COINSELECTION_H_
