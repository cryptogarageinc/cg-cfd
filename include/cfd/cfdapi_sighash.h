// Copyright 2019 CryptoGarage
/**
 * @file cfdapi_sighash.h
 *
 * @brief cfd-apiで利用するSigHashのクラス定義
 *
 * JSON形式のAPIを提供する.
 */
#ifndef CFD_INCLUDE_CFD_CFDAPI_SIGHASH_H_
#define CFD_INCLUDE_CFD_CFDAPI_SIGHASH_H_

#include <string>

#include "cfd/cfd_common.h"
#include "cfd/cfdapi_struct.h"

/**
 * @brief cfdapi名前空間
 */
namespace cfd {
namespace api {

/**
 * @brief SigHash関連のJSON APIクラス
 */
class CFD_EXPORT SigHashApi {
 public:
  /**
   * @brief JSONパラメータの情報を元に、SigHashを作成する
   * @param[in] request sighashを生成するパラメータ
   * @return sighashのhexデータを格納した構造体
   */
  static CreateSignatureHashResponseStruct CreateSignatureHash(
      const CreateSignatureHashRequestStruct& request);

 private:
  SigHashApi();
};

}  // namespace api
}  // namespace cfd

#endif  // CFD_INCLUDE_CFD_CFDAPI_SIGHASH_H_
