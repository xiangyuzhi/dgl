/**
 *  Copyright (c) 2023 by Contributors
 * @file cnumpy.h
 * @brief Numpy File Fetecher class.
 */

#include <stdint.h>
#include <torch/script.h>
#include <zlib.h>

#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <typeinfo>

namespace graphbolt {
namespace storage {

/**
 * @brief Disk Numpy Fetecher class.
 */
class OnDiskNpyArray {
 public:
  /** @brief Constructor with empty file path. */
  OnDiskNpyArray() : word_size(0), prefix_len(0), feat_dim(0) {}

  /** @brief Constructor with given file path. */
  OnDiskNpyArray(std::string _filename) : filename(_filename) {
    FILE *fp = fopen(_filename.c_str(), "rb");
    if (!fp)
      throw std::runtime_error("npy_load: Unable to open file " + _filename);
    parse_npy_header(fp);
    fclose(fp);
  }

  /**
   * @brief Parse numpy meta data.
   */
  void parse_npy_header(FILE *fp);

  /**
   * @brief Get the feature shape of numpy data according to meta data.
   */
  torch::Tensor feature_shape() { return feat_shape; }

  /**
   * @brief Read disk numpy file based on given index and transform to
   * tensor.
   */
  torch::Tensor index_select_iouring(torch::Tensor idx);

 private:
  std::string filename;      // Path to numpy file.
  torch::Tensor feat_shape;  // Shape of features, e.g. (N,M,K,L).
  signed long feat_dim;      // Sum dim of a single feature, e.g. M*K*L.
  size_t word_size;          // Number of bytes of a feature element.
  size_t prefix_len;         // Length of head data in numpy file.
};

}  // namespace storage
}  // namespace graphbolt