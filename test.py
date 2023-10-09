import prunedbpe

bpe = prunedbpe.learn("bas_clean.txt", -1,2048)

bpe.save("bas_clean.bpe")
