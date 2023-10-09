# Byte-level BPE with pruning

Byte pair encodings are suboptimal. Once a new token is created because of its frequency, it is not guaranteed to keep that frequency until finishing the BPE, because other pairs involving this token might arise, which will decrease the frequency of the token. A solution to this is dynamically pruning intermediate tokens whose frequency has dropped below a certain threshold during creating the BPE. Pruning creates a better encoding whose tokens all adhere to the frequency threshold, which frees up tokens
Pruning frees up tokens to encode other pairs, which leads to a more efficient encoding. 
It also guarantees that each token will be seen at least min_frequency times in the dataset, so that the model can better learn the different contexts in which the token is used.
the frequencies need to be compared after they have been subtracted from the frequencies!
also, when deleting a token, add the frequencies back to the tokens it used to replace.
this could lead to an instability and an infinite loop. we need to see. i think not, since we removed the tokens that led to the token in the first place. 

i have an idea for a better BPE. imagine during creation of the encoding, you discover a token A=BC that is used 99 times. then we find another token D = AE = BCE which is used 50 times. so the remaining usages of A would be 49. so if we delete A from our set of tokens, and add D to our set of tokens, the encoding length of our whole text would still shrink by 1, without increasing the number of tokens, because we replaced 50 BCEs with D, and added 49 BCs for where A was