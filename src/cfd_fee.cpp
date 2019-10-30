// Copyright 2019 CryptoGarage
/**
 * @file cfd_fee.cpp
 *
 * @brief Fee計算の関連クラスの実装ファイル
 */
#include <algorithm>
#include <string>
#include <vector>

#include "cfd/cfd_fee.h"
#include "cfdcore/cfdcore_amount.h"

namespace cfd {

using cfd::core::Amount;

// -----------------------------------------------------------------------------
// ファイル内関数
// -----------------------------------------------------------------------------
#if 0
struct TxConfirmStats {
  uint32_t max_periods;
  double decay;
  uint32_t scale;

  uint32_t GetMaxConfirms() { return max_periods * scale; }
}

// returns -1 on error conditions
static double EstimateMedianVal(int confTarget, double sufficientTxVal,
    double successBreakPoint, bool requireGreater,
    unsigned int nBlockHeight, const TxConfirmStats& stats) {
  // Counters for a bucket (or range of buckets)
  double nConf = 0;
  double totalNum = 0;
  int extraNum = 0;
  double failNum = 0;
  int periodTarget = (confTarget + scale - 1)/scale;

  int maxbucketindex = buckets.size() - 1;

  // NOLINT requireGreater means we are looking for the lowest feerate such that all higher
  // NOLINT values pass, so we start at maxbucketindex (highest feerate) and look at successively
  // NOLINT smaller buckets until we reach failure.  Otherwise, we are looking for the highest
  // NOLINT feerate such that all lower values fail, and we go in the opposite direction.
  unsigned int startbucket = requireGreater ? maxbucketindex : 0;
  int step = requireGreater ? -1 : 1;

  // We'll combine buckets until we have enough samples.
  // The near and far variables will define the range we've combined
  // The best variables are the last range we saw which still had a high
  // enough confirmation rate to count as success.
  // The cur variables are the current range we're counting.
  unsigned int curNearBucket = startbucket;
  unsigned int bestNearBucket = startbucket;
  unsigned int curFarBucket = startbucket;
  unsigned int bestFarBucket = startbucket;

  bool foundAnswer = false;
  unsigned int bins = unconfTxs.size();
  bool newBucketRange = true;
  bool passing = true;

  // Start counting from highest(default) or lowest feerate transactions
  for (int bucket = startbucket; bucket >= 0 && bucket <= maxbucketindex;
        bucket += step) {
    if (newBucketRange) {
      curNearBucket = bucket;
      newBucketRange = false;
    }
    curFarBucket = bucket;
    nConf += confAvg[periodTarget - 1][bucket];
    totalNum += txCtAvg[bucket];
    failNum += failAvg[periodTarget - 1][bucket];
    for (unsigned int confct = confTarget;
        confct < stats.GetMaxConfirms(); confct++)
      extraNum += unconfTxs[(nBlockHeight - confct)%bins][bucket];
    extraNum += oldUnconfTxs[bucket];
    // NOLINT If we have enough transaction data points in this range of buckets,
    // NOLINT we can test for success
    // NOLINT (Only count the confirmed data points, so that each confirmation count
    // NOLINT will be looking at the same amount of data and same bucket breaks)
    if (totalNum >= sufficientTxVal / (1 - decay)) {
      double curPct = nConf / (totalNum + failNum + extraNum);

      // NOLINT Check to see if we are no longer getting confirmed at the success rate
      if ((requireGreater && curPct < successBreakPoint) ||
            (!requireGreater && curPct > successBreakPoint)) {
        if (passing == true) {
          // First time we hit a failure record the failed bucket
          passing = false;
        }
        continue;
      } else {
        // Otherwise update the cumulative stats, and the bucket variables
        // and reset the counters
        // Reset any failed bucket, currently passing
      }
    }
  }

  double median = -1;
  double txSum = 0;

  // NOLINT Calculate the "average" feerate of the best bucket range that met success conditions
  // NOLINT Find the bucket with the median transaction and then report the average feerate from that bucket
  // NOLINT This is a compromise between finding the median which we can't since we don't save all tx's
  // and reporting the average which is less accurate
  unsigned int minBucket = std::min(bestNearBucket, bestFarBucket);
  unsigned int maxBucket = std::max(bestNearBucket, bestFarBucket);
  for (unsigned int j = minBucket; j <= maxBucket; j++) {
    txSum += txCtAvg[j];
  }
  if (foundAnswer && txSum != 0) {
    txSum = txSum / 2;
    for (unsigned int j = minBucket; j <= maxBucket; j++) {
      if (txCtAvg[j] < txSum) {
        txSum -= txCtAvg[j];
      } else {  // we're in the right bucket
        median = avg[j] / txCtAvg[j];
        break;
      }
    }
  }

  return median;
}

/** Return a fee estimate at the required successThreshold from the shortest
 * time horizon which tracks confirmations up to the desired target.  If
 * checkShorterHorizon is requested, also allow short time horizon estimates
 * for a lower target to reduce the given answer */
static double EstimateCombinedFee(uint32_t conf_target,
    double successThreshold, bool checkShorterHorizon) {
  static constexpr uint32_t kFeeMaxConfirm = 24 * 2;
  static constexpr uint32_t kShortMaxConfirm = 12 * 1;
  static constexpr uint32_t kLongMaxConfirm = 42 * 24;
  // feeStats = std::unique_ptr<TxConfirmStats>(new TxConfirmStats(buckets,
  //  bucketMap, MED_BLOCK_PERIODS, MED_DECAY, MED_SCALE));
  // shortStats = std::unique_ptr<TxConfirmStats>(new TxConfirmStats(buckets,
  //  bucketMap, SHORT_BLOCK_PERIODS, SHORT_DECAY, SHORT_SCALE));
  // longStats = std::unique_ptr<TxConfirmStats>(new TxConfirmStats(buckets,
  //  bucketMap, LONG_BLOCK_PERIODS, LONG_DECAY, LONG_SCALE));
  // unsigned int maxPeriods, double _decay, unsigned int _scale
  // GetMaxConfirms() => maxPeriods * _scale

  double estimate = -1;
  if (conf_target >= 1 && conf_target <= longStats->GetMaxConfirms()) {
    // Find estimate from shortest time horizon possible
    if (conf_target <= shortStats->GetMaxConfirms()) {  // short horizon
      estimate = shortStats->EstimateMedianVal(conf_target,
          SUFFICIENT_TXS_SHORT, successThreshold, true, nBestSeenHeight);
    } else if (conf_target <= kFeeMaxConfirm) {  // medium horizon
      estimate = feeStats->EstimateMedianVal(conf_target, SUFFICIENT_FEETXS,
          successThreshold, true, nBestSeenHeight);
    } else {  // long horizon
      estimate = longStats->EstimateMedianVal(conf_target, SUFFICIENT_FEETXS,
            successThreshold, true, nBestSeenHeight);
    }
    if (checkShorterHorizon) {
      // NOLINT If a lower conf_target from a more recent horizon returns a lower answer use it.
      if (conf_target > kFeeMaxConfirm) {
        double medMax = feeStats->EstimateMedianVal(kFeeMaxConfirm,
            SUFFICIENT_FEETXS, successThreshold, true, nBestSeenHeight);
        if (medMax > 0 && (estimate == -1 || medMax < estimate)) {
          estimate = medMax;
        }
      }
      if (conf_target > shortStats->GetMaxConfirms()) {
        double shortMax = shortStats->EstimateMedianVal(
            shortStats->GetMaxConfirms(), SUFFICIENT_TXS_SHORT,
            successThreshold, true, nBestSeenHeight);
        if (shortMax > 0 && (estimate == -1 || shortMax < estimate)) {
          estimate = shortMax;
        }
      }
    }
  }
  return estimate;
}

static int64_t EstimateSmartFee(uint32_t confirm_target) {
  static constexpr double kHalfSuccessPct = .6;
  static constexpr double kSuccessPct = .85;
  static constexpr double kDoubleSuccessPct = .95;
  uint32_t conf_target = confirm_target;
  double median = -1;
  EstimationResult tempResult;

  // Return failure if trying to analyze a target we're not tracking
  if (conf_target <= 0 ||
      (unsigned int)conf_target > longStats->GetMaxConfirms()) {
    return 0;  // error condition
  }

  // It's not possible to get reasonable estimates for conf_target of 1
  if (conf_target == 1) conf_target = 2;

  unsigned int maxUsableEstimate = MaxUsableEstimate();
  if ((unsigned int)conf_target > maxUsableEstimate) {
    conf_target = maxUsableEstimate;
  }

  if (conf_target <= 1) return 0;  // error condition

  /** true is passed to estimateCombined fee for target/2 and target so
   * that we check the max confirms for shorter time horizons as well.
   * This is necessary to preserve monotonically increasing estimates.
   * For non-conservative estimates we do the same thing for 2*target, but
   * for conservative estimates we want to skip these shorter horizons
   * checks for 2*target because we are taking the max over all time
   * horizons so we already have monotonically increasing estimates and
   * the purpose of conservative estimates is not to let short term
   * fluctuations lower our estimates by too much.
   */
  double halfEst = EstimateCombinedFee(conf_target/2, kHalfSuccessPct, true);
  median = halfEst;
  double actualEst = EstimateCombinedFee(conf_target, kSuccessPct, true);
  if (actualEst > median) {
    median = actualEst;
  }
  double doubleEst = EstimateCombinedFee(2 * conf_target, kDoubleSuccessPct,
      true);
  if (doubleEst > median) {
    median = doubleEst;
  }

  if (median < 0) return 0;  // error condition
  return llround(median);
}
#endif

/**
 * @brief calculate fee
 * @param[in] confirm_target    confirm target
 * @return fee rate
 */
static int64_t EstimateSmartFee(uint32_t confirm_target) {
  // TODO(k-matsuzawa): 暫定。移植はwallet or nodeの動的パラメータが多すぎる
  return static_cast<int64_t>(confirm_target);
}

// -----------------------------------------------------------------------------
// FeeCalculator
// -----------------------------------------------------------------------------
Amount FeeCalculator::CalculateFee(  // Fee計算
    uint32_t size, uint32_t vsize, uint32_t rate) {
  // 簡易計算
  int64_t satoshi;
  if (size == vsize) {
    satoshi = size * rate / 1000;
  } else {
    satoshi = vsize * rate / 1000;
  }
  if (satoshi < kRelayMinimumFee) {
    satoshi = kRelayMinimumFee;
  }
  return Amount::CreateBySatoshiAmount(satoshi);
}

uint64_t FeeCalculator::GetMinimumFeeRate() {
  static constexpr uint32_t kConfirmBlockNum = 1008;
  // static constexpr int64_t kMempoolMinimumFee = 300 * 1000000;
  // confirm_target: 1008
  // wallet.chain().estimateSmartFee(target, conservative_estimate, feeCalc);
  /* User control of how to calculate fee uses the following parameter precedence:
     1. coin_control.m_feerate
     2. coin_control.m_confirm_target
     3. m_pay_tx_fee (user-set member variable of wallet)
     4. m_confirm_target (user-set member variable of wallet)
     The first parameter that is set is used.
  */
  int64_t fee = 0;
  // -> 2,4のみ適用
  // NOLINT feerate_needed = wallet.chain().estimateSmartFee(target, conservative_estimate, feeCalc);
  fee = EstimateSmartFee(kConfirmBlockNum);

  // Obey mempool min fee when using smart fee estimation
  // CFeeRate min_mempool_feerate = wallet.chain().mempoolMinFee();
  // NOLINT -> return ::mempool.GetMinFee(gArgs.GetArg("-maxmempool", DEFAULT_MAX_MEMPOOL_SIZE) * 1000000);
  // if (kMempoolMinimumFee > fee) {
  //   fee = kMempoolMinimumFee;
  // }

  // prevent user from paying a fee below the required fee rate
  // CFeeRate required_feerate = GetRequiredFeeRate(wallet);
  if (kRelayMinimumFee > fee) {
    fee = kRelayMinimumFee;
  }
  return static_cast<uint64_t>(fee);
}

FeeCalculator::FeeCalculator() : FeeCalculator(1000) {}

FeeCalculator::FeeCalculator(uint64_t baserate) : baserate_(baserate) {}

Amount FeeCalculator::GetFee(uint32_t size) const {
  return GetFee(static_cast<size_t>(size));
}

Amount FeeCalculator::GetFee(size_t size) const {
  int64_t byte_size = static_cast<int64_t>(size);
  int64_t fee = baserate_ * byte_size / 1000;
  if ((fee == 0) && (byte_size != 0) && (baserate_ != 0)) {
    fee = 1;
  }
  return Amount::CreateBySatoshiAmount(fee);
}

Amount FeeCalculator::GetFee(const Utxo& utxo) const {
  uint32_t minimum_txin = static_cast<uint32_t>(TxIn::kMinimumTxInSize);
  uint32_t size = minimum_txin + utxo.uscript_size_max + utxo.witness_size_max;
  return GetFee(size);
}

}  // namespace cfd
