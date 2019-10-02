// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_hdwallet.h
 *
 * @brief cfd-apiで利用するHDWallet APIのクラス定義
 *
 * 構造体形式のAPIを提供する.
 */
#ifndef CFD_INCLUDE_CFD_CFDAPI_HDWALLET_H_
#define CFD_INCLUDE_CFD_CFDAPI_HDWALLET_H_

#include "cfd/cfd_common.h"
#include "cfd/cfdapi_struct.h"

/**
 * @brief cfdapi名前空間
 */
namespace cfd {
namespace api {

/**
 * @brief 鍵情報関連の関数群クラス
 */
class CFD_EXPORT HDWalletApi {
 public:
  /**
   * @brief JSONパラメータの情報を元に、BIP39 で利用できる Wordlist を取得する.
   * @param[in] request Wordlistの言語を含むリクエスト構造体
   * @return Wordlist一覧を含むレスポンス構造体
   */
  static GetMnemonicWordlistResponseStruct GetMnemonicWordlist(
      const GetMnemonicWordlistRequestStruct& request);

 private:
  HDWalletApi();
};

}  // namespace api
}  // namespace cfd

#endif  // CFD_INCLUDE_CFD_CFDAPI_HDWALLET_H_
