import numpy as np 
from pyOpenFHE import CKKS
from pyOpenFHE import enums as pal
from math import log2

np.set_printoptions(formatter={'float': lambda x: "{0:0.3f}".format(x)})

mult_depth = 26
scale_factor_bits = 59
batch_size = 64

# Pass the non-default parameters
cc = CKKS.genCryptoContextCKKS(
  mult_depth, 
  scale_factor_bits,
  batch_size,
  stdLevel=pal.SecurityLevel.HEStd_NotSet, ringDim=128
)

# enable features
cc.enable(pal.PKESchemeFeature.PKE)
cc.enable(pal.PKESchemeFeature.KEYSWITCH)
cc.enable(pal.PKESchemeFeature.LEVELEDSHE)
cc.enable(pal.PKESchemeFeature.ADVANCEDSHE)
cc.enable(pal.PKESchemeFeature.FHE)

# generate keys
keys = cc.keyGen()
cc.evalMultKeyGen(keys.secretKey)
cc.evalPowerOf2RotationKeyGen(keys.secretKey)

cc.evalBootstrapSetup()
cc.evalBootstrapKeyGen(keys.secretKey)

x1 = np.random.random(batch_size)
x2 = np.random.random(batch_size)

c1 = cc.encrypt(keys.publicKey, x1)
c2 = cc.encrypt(keys.publicKey, x2)

for i in range(23):
    c1 = c1 * 1.0 + 1. - c2

c3 = c1
for j in range(10):
    c3 = cc.evalBootstrap(c3)
    for i in range(3):
        print(c3.getTowersRemaining())
        c3 = c3 * 1.0 + 1. - c2


y3 = cc.decrypt(keys.secretKey, c3)
print(y3)
