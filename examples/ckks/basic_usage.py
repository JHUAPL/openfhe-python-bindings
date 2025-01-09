import numpy as np 
from pyOpenFHE import CKKS
from pyOpenFHE import enums as pal
from math import log2

np.set_printoptions(formatter={'float': lambda x: "{0:0.3f}".format(x)})

mult_depth = 5
scale_factor_bits = 59
batch_size = 16

# Pass the non-default parameters
cc = CKKS.genCryptoContextCKKS(
  mult_depth, 
  scale_factor_bits,
  batch_size
)

# enable features
cc.enable(pal.PKESchemeFeature.PKE)
cc.enable(pal.PKESchemeFeature.KEYSWITCH)
cc.enable(pal.PKESchemeFeature.LEVELEDSHE)
cc.enable(pal.PKESchemeFeature.ADVANCEDSHE)

# generate keys
keys = cc.keyGen()
cc.evalMultKeyGen(keys.secretKey)
cc.evalPowerOf2RotationKeyGen(keys.secretKey)



x1 = np.random.random(batch_size)
x2 = np.random.random(batch_size)

c1 = cc.encrypt(keys.publicKey, x1)
c2 = cc.encrypt(keys.publicKey, x2)

y1 = cc.decrypt(keys.secretKey, c1)
print(y1)
