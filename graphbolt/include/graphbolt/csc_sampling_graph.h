/**
 *  Copyright (c) 2023 by Contributors
 * @file graphbolt/csc_sampling_graph.h
 * @brief Header file of csc sampling graph.
 */
#ifndef GRAPHBOLT_CSC_SAMPLING_GRAPH_H_
#define GRAPHBOLT_CSC_SAMPLING_GRAPH_H_

#include <graphbolt/sampled_subgraph.h>

#include <string>
#include <vector>

#include "sampled_subgraph.h"

namespace graphbolt {
namespace sampling {

/**
 * @brief A sampling oriented csc format graph.
 *
 * Example usage:
 *
 * Suppose the graph has 3 node types, 3 edge types and 6 edges
 * auto node_type_offset = {0, 2, 4, 6}
 * auto type_per_edge = {0, 1, 0, 2, 1, 2}
 * auto graph = CSCSamplingGraph(..., ..., node_type_offset, type_per_edge)
 *
 * The `node_type_offset` tensor represents the offset array of node type, the
 * given array indicates that node [0, 2) has type id 0, [2, 4) has type id 1,
 * and [4, 6) has type id 2. And the `type_per_edge` tensor represents the type
 * id of each edge.
 */
class CSCSamplingGraph : public torch::CustomClassHolder {
 public:
  /** @brief Default constructor. */
  CSCSamplingGraph() = default;

  /**
   * @brief Constructor for CSC with data.
   * @param indptr The CSC format index pointer array.
   * @param indices The CSC format index array.
   * @param node_type_offset A tensor representing the offset of node types, if
   * present.
   * @param type_per_edge A tensor representing the type of each edge, if
   * present.
   */
  CSCSamplingGraph(
      torch::Tensor& indptr, torch::Tensor& indices,
      const torch::optional<torch::Tensor>& node_type_offset,
      const torch::optional<torch::Tensor>& type_per_edge);

  /**
   * @brief Create a homogeneous CSC graph from tensors of CSC format.
   * @param indptr Index pointer array of the CSC.
   * @param indices Indices array of the CSC.
   * @param node_type_offset A tensor representing the offset of node types, if
   * present.
   * @param type_per_edge A tensor representing the type of each edge, if
   * present.
   *
   * @return CSCSamplingGraph
   */
  static c10::intrusive_ptr<CSCSamplingGraph> FromCSC(
      torch::Tensor indptr, torch::Tensor indices,
      const torch::optional<torch::Tensor>& node_type_offset,
      const torch::optional<torch::Tensor>& type_per_edge);

  /** @brief Get the number of nodes. */
  int64_t NumNodes() const { return indptr_.size(0) - 1; }

  /** @brief Get the number of edges. */
  int64_t NumEdges() const { return indices_.size(0); }

  /** @brief Get the csc index pointer tensor. */
  const torch::Tensor CSCIndptr() const { return indptr_; }

  /** @brief Get the index tensor. */
  const torch::Tensor Indices() const { return indices_; }

  /** @brief Get the node type offset tensor for a heterogeneous graph. */
  inline const torch::optional<torch::Tensor> NodeTypeOffset() const {
    return node_type_offset_;
  }

  /** @brief Get the edge type tensor for a heterogeneous graph. */
  inline const torch::optional<torch::Tensor> TypePerEdge() const {
    return type_per_edge_;
  }

  /**
   * @brief Magic number to indicate graph version in serialize/deserialize
   * stage.
   */
  static constexpr int64_t kCSCSamplingGraphSerializeMagic = 0xDD2E60F0F6B4A128;

  /**
   * @brief Load graph from stream.
   * @param archive Input stream for deserializing.
   */
  void Load(torch::serialize::InputArchive& archive);

  /**
   * @brief Save graph to stream.
   * @param archive Output stream for serializing.
   */
  void Save(torch::serialize::OutputArchive& archive) const;

  /**
   * @brief Return the subgraph induced on the inbound edges of the given nodes.
   * @param nodes Type agnostic node IDs to form the subgraph.
   *
   * @return SampledSubgraph.
   */
  c10::intrusive_ptr<SampledSubgraph> InSubgraph(
      const torch::Tensor& nodes) const;

 private:
  /** @brief CSC format index pointer array. */
  torch::Tensor indptr_;

  /** @brief CSC format index array. */
  torch::Tensor indices_;

  /**
   * @brief Offset array of node type. The length of it is equal to the number
   * of node types + 1. The tensor is in ascending order as nodes of the same
   * type have continuous IDs, and larger node IDs are paired with larger node
   * type IDs. Its first value is 0 and last value is the number of nodes. And
   * nodes with ID between `node_type_offset_[i] ~ node_type_offset_[i+1]` are
   * of type id `i`.
   */
  torch::optional<torch::Tensor> node_type_offset_;

  /**
   * @brief Type id of each edge, where type id is the corresponding index of
   * edge types. The length of it is equal to the number of edges.
   */
  torch::optional<torch::Tensor> type_per_edge_;
};

}  // namespace sampling
}  // namespace graphbolt

#endif  // GRAPHBOLT_CSC_SAMPLING_GRAPH_H_
