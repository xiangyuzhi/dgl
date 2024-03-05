/**
 *  Copyright (c) 2023 by Contributors
 * @file index_select.cc
 * @brief Index select operators.
 */
#include <graphbolt/cuda_ops.h>
#include <graphbolt/fused_csc_sampling_graph.h>

#include "./cnumpy.h"
#include "./macro.h"
#include "./utils.h"

namespace graphbolt {
namespace ops {

torch::Tensor IndexSelect(torch::Tensor input, torch::Tensor index) {
  if (utils::is_on_gpu(index) && input.is_pinned()) {
    GRAPHBOLT_DISPATCH_CUDA_ONLY_DEVICE(
        c10::DeviceType::CUDA, "UVAIndexSelect",
        { return UVAIndexSelectImpl(input, index); });
  }
  return input.index({index.to(torch::kLong)});
}

torch::Tensor DiskIndexSelect(
    std::string path, torch::Tensor index, torch::ScalarType dtype) {
  storage::OnDiskNpyArray arr(path);
  return arr.index_select_iouring({index.to(torch::kLong)}, dtype);
}

torch::Tensor DiskFeatureShape(std::string path) {
  storage::OnDiskNpyArray arr(path);
  return arr.feature_shape();
}

std::tuple<torch::Tensor, torch::Tensor> IndexSelectCSC(
    torch::Tensor indptr, torch::Tensor indices, torch::Tensor nodes,
    torch::optional<int64_t> output_size) {
  TORCH_CHECK(
      indices.sizes().size() == 1, "IndexSelectCSC only supports 1d tensors");
  if (utils::is_on_gpu(nodes) && utils::is_accessible_from_gpu(indptr) &&
      utils::is_accessible_from_gpu(indices)) {
    GRAPHBOLT_DISPATCH_CUDA_ONLY_DEVICE(
        c10::DeviceType::CUDA, "IndexSelectCSCImpl",
        { return IndexSelectCSCImpl(indptr, indices, nodes, output_size); });
  }
  // @todo: The CPU supports only integer dtypes for indices tensor.
  TORCH_CHECK(
      c10::isIntegralType(indices.scalar_type(), false),
      "IndexSelectCSC is not implemented to slice noninteger types yet.");
  sampling::FusedCSCSamplingGraph g(indptr, indices);
  const auto res = g.InSubgraph(nodes);
  return std::make_tuple(res->indptr, res->indices);
}

}  // namespace ops
}  // namespace graphbolt
