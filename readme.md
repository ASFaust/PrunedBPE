# PrunedBPE: Byte-level BPE with Dynamic Pruning

## Overview
PrunedBPE is a Python library implemented in C++ using pybind11. It introduces an innovative approach to Byte Pair Encoding (BPE) by dynamically pruning intermediate tokens. This method ensures that each token consistently meets a specified frequency threshold throughout the BPE process.

## Installation
To install PrunedBPE, run the following command in the root directory of the repository:

```
pip3 install .
```

## Concept
Traditional BPE has a limitation: once a token is created due to its initial high frequency, it is not reassessed for frequency as the encoding process continues. This can lead to suboptimal encodings. PrunedBPE addresses this issue by dynamically pruning tokens whose frequency falls below a certain threshold during the BPE process.

### Advantages
- **Efficient Encoding**: By pruning less frequent tokens, PrunedBPE allows for the allocation of tokens to more frequently occurring pairs, leading to a more efficient encoding.
- **Frequency Guarantee**: Each token in the final encoding is guaranteed to appear at least a minimum number of times (`min_frequency`), facilitating better learning of token contexts in the dataset.

### Implementation Details
- **Frequency Adjustment**: When a token is pruned, its frequency count is redistributed to the tokens it replaced. This approach ensures the stability of the encoding process.
- **Loop Prevention**: The algorithm avoids potential infinite loops by removing tokens that contribute to the creation of a new, pruned token.

## Example
Consider the following scenario in BPE:
- Token A (`BC`) occurs 99 times.
- A new token D (`AE` or `BCE`) is found to occur 50 times.

In traditional BPE, A would remain in the set. However, in PrunedBPE, A is pruned and replaced by D. This results in:
- 50 instances of `BCE` being replaced by D.
- The remaining 49 instances of A (`BC`) are added back.

This approach maintains the encoding length while ensuring a more efficient and contextually relevant set of tokens.

---

*Note: The above documentation focuses on the implementation and advantages of the PrunedBPE approach, aligning with the library's functionality.*
