def hexify(d):
    return '\t'.join([hex(e) for e in d])

class SHA(object):
    def __init__(self, mode="SHA256"):  # Defaults set for SHA1
        ## Default Values for all functions:
        # These can be overridden to arbitrarily augment SHA, given defaults conform to spec.

        #W Generator Feedback Taps - distance back from most recent term
        self.W_SUMS = [7,16] #these taps are all added together.
        self.P_SIGMA_LOW_0 = 15
        self.P_SIGMA_LOW_1 = 2

        # Default Initial Hash Value constants
        self.SHA224_H0 =   [0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939, 0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4]
        self.SHA256_H0 =   [0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19]
        self.SHA384_H0 =   [0xcbbb9d5dc1059ed8, 0x629a292a367cd507, 0x9159015a3070dd17, 0x152fecd8f70e5939, 
                            0x67332667ffc00b31, 0x8eb44a8768581511, 0xdb0c2e0d64f98fa7, 0x47b5481dbefa4fa4]
        self.SHA512_H0 =   [0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1, 
                            0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179]

        # SHA-224, SHA-256:
        self.SIGMA_CAP_256_0 = lambda x: self.Sigma_cap(x, 2, 13, 22)
        self.SIGMA_CAP_256_1 = lambda x: self.Sigma_cap(x, 6, 11, 25)
        self.SIGMA_LOW_256_0 = lambda x: self.Sigma_low(x, 7, 18, 3)
        self.SIGMA_LOW_256_1 = lambda x: self.Sigma_low(x, 17, 19, 10)
        self.K_256 = [
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2]

        ## SHA-384, SHA-512:
        self.SIGMA_CAP_512_0 = lambda x: self.Sigma_cap(x, 28, 34, 39)
        self.SIGMA_CAP_512_1 = lambda x: self.Sigma_cap(x, 14, 18, 41)
        self.SIGMA_LOW_512_0 = lambda x: self.Sigma_low(x, 1, 8, 7)
        self.SIGMA_LOW_512_1 = lambda x: self.Sigma_low(x, 19, 61, 6)
        self.K_512 = [
            0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
            0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
            0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
            0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
            0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
            0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
            0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
            0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
            0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
            0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
            0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
            0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
            0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
            0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
            0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
            0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
            0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
            0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
            0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
            0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817]
        
        ## Set function specifics based on mode.
        if mode=="SHA224":
            self.WORDSIZE = 32
            self.BLOCKSIZE = 512
            self.ITERATIONS = 64
            self.DIGEST_BITS = 224
            self.H0 = self.SHA224_H0
        elif mode=="SHA256":
            self.WORDSIZE = 32
            self.BLOCKSIZE = 512
            self.ITERATIONS = 64
            self.DIGEST_BITS = 256
            self.H0 = self.SHA256_H0
        elif mode=="SHA384":
            self.WORDSIZE = 64
            self.BLOCKSIZE = 1024
            self.ITERATIONS = 80
            self.DIGEST_BITS = 384
            self.H0 = self.SHA384_H0
        elif mode=="SHA512":
            self.WORDSIZE = 64
            self.BLOCKSIZE = 1024
            self.ITERATIONS = 80
            self.DIGEST_BITS = 512
            self.H0 = self.SHA512_H0
        
        #Some parameters are grouped by size:
        if mode=="SHA224" or mode=="SHA256":
            self.K = self.K_256
            self.SIGMA_CAP_0 = self.SIGMA_CAP_256_0
            self.SIGMA_CAP_1 = self.SIGMA_CAP_256_1
            self.SIGMA_LOW_0 = self.SIGMA_LOW_256_0
            self.SIGMA_LOW_1 = self.SIGMA_LOW_256_1
        elif mode=="SHA384" or mode=="SHA512":
            self.K = self.K_512
            self.SIGMA_CAP_0 = self.SIGMA_CAP_512_0
            self.SIGMA_CAP_1 = self.SIGMA_CAP_512_1
            self.SIGMA_LOW_0 = self.SIGMA_LOW_512_0
            self.SIGMA_LOW_1 = self.SIGMA_LOW_512_1


    ### Process Algorithms - while you could override these, you probably don't want/need to.

    ## Bitwise helpers
    def rotl(self, x, n):
        w=self.WORDSIZE
        n=n%w
        return ((x<<n) | (x>>w-n)) & (2**w -1)

    def rotr(self, x, n):
        w=self.WORDSIZE
        n=n%w
        return ((x>>n) | (x<<w-n)) & (2**w -1)

    def shr(self, x, n):
        return (x>>n)

    def crop(self, x):
        w=self.WORDSIZE
        return x & (2**w -1)


    ## Generic SHA functions
    def Ch(self,x,y,z):
        return (x&y)^(~x&z)

    def Parity(self,x,y,z):
        return x^y^z

    def Maj(self,x,y,z):
        return (x&y)^(x&z)^(y&z)

    def Sigma_cap(self, x,A,B,C):
        return self.rotr(x, A) ^ self.rotr(x, B) ^ self.rotr(x, C)

    def Sigma_low(self, x,A,B,C):
        return self.rotr(x, A) ^ self.rotr(x, B) ^ self.shr(x, C)


    #Pad to 512 or 1024 bytes, return padded message.
    #Message m must be Python3 bytes
    #Currently only supports messages with lengths that are a multiple of 8 bits.
    def pad(self, m):
        l = len(m)
        l_bits = l*8
        m += b"\x80"
        if (self.BLOCKSIZE == 512):
            k = (56-1-l)%64 #Number of pad bytes
            pad = b"\x00" * k
            pad += l_bits.to_bytes(8, byteorder='big')
        elif (self.BLOCKSIZE == 1024):
            k = (112-1-l)%128
            pad = b"\x00" * k
            pad += l_bits.to_bytes(16, byteorder='big')
        else:
            return None
        return m+pad

    # Main SHA computation
    def sha_main(self, m): 
        H = self.H0.copy()
        K = self.K
        Sigma_cap_0 = self.SIGMA_CAP_0
        Sigma_cap_1 = self.SIGMA_CAP_1
        Sigma_low_0 = self.SIGMA_LOW_0
        Sigma_low_1 = self.SIGMA_LOW_1
        m = self.pad(m)
        ws = self.WORDSIZE//8
        M = [m[i:i+self.BLOCKSIZE//8] for i in range(0, len(m), self.BLOCKSIZE//8)]
        W = [0]*self.ITERATIONS

        self.last_intermediate_results = [] #Used for generating test benches
        self.last_blocks = M #Used for generating test benches
        for i in range(len(M)):
            #1: Prepare the message schedule
            for t in range(self.ITERATIONS):
                if t<16:
                    W[t] = int.from_bytes( M[i][t*ws: (t+1)*ws], byteorder="big")
                else:
                    W[t] = self.crop( Sigma_low_1(W[t-self.P_SIGMA_LOW_1])
                                    + Sigma_low_0(W[t-self.P_SIGMA_LOW_0])
                                    + sum([W[t-tap] for tap in self.W_SUMS]) )

            #2: Initialize 5 working variables
            a=H[0]
            b=H[1]
            c=H[2]
            d=H[3]
            e=H[4]
            f=H[5]
            g=H[6]
            h=H[7]

            #3
            for t in range(self.ITERATIONS):
                temp1 = self.crop( h + Sigma_cap_1(e) + self.Ch(e,f,g) + K[t] + W[t] )
                temp2 = self.crop( Sigma_cap_0(a) + self.Maj(a,b,c) )
                h=g
                g=f
                f=e
                e=self.crop(d+temp1)
                d=c
                c=b
                b=a
                a=self.crop(temp1+temp2)

            #4 Compute the ith intermediate hash value H(i)
            H[0] = self.crop(H[0] + a)
            H[1] = self.crop(H[1] + b)
            H[2] = self.crop(H[2] + c)
            H[3] = self.crop(H[3] + d)
            H[4] = self.crop(H[4] + e)
            H[5] = self.crop(H[5] + f)
            H[6] = self.crop(H[6] + g)
            H[7] = self.crop(H[7] + h)

            self.last_intermediate_results += [b''.join([H[z].to_bytes(ws,byteorder="big") for z in range(8)])]

        #Final digest:
        return self.last_intermediate_results[-1]

    def digest(self, m):
        return self.sha_main(m)[:(self.DIGEST_BITS//8)]

#Arbitrarily Augment the SHA algorithm
class SHAAA(SHA):
    def __init__(self, config={}, basetype=None): #Configure from scratch
        """Secure Hash Algorithm - Arbitrarily Augmented

        For use with SHAv2 style variants (224,256,384,512, etc.)
        basetype -- Start with parameters used for a specific algorithm ("SHA224", "SHA256", "SHA384", "SHA512")
        config -- Dictionary of parameters, below. All 9 required unless basetype is specifid.
        
        Available parameters:
        WORDSIZE:       32,64
        BLOCKSIZE:      512,1024   #Currently must be 16*WORDSIZE
        DIGESTBITS:     int, max 8*WORDSIZE.  #Cropped output size. (256 for SHA256, etc.)
        ITERATIONS:     >=1
        H0:             [WORD]x8
        K:              [WORD]xITERATIONS
        W_SUMS:         
        P_SIGMA_LOW_<0,1>:     int, max WORDSIZE, which parameter to use in SIGMA_LOW functions
        SIGMA_<LOW,CAP>_<0,1>: Tuple of 3 ints (max WORDSIZE) for SIGMA functions
        """
        SHA.__init__(self, mode=basetype)

        for key in config:
            if key[:9] == "SIGMA_CAP":
                setattr(self, key, lambda x, p=config[key]: self.Sigma_cap(x, *p))
            elif key[:9] == "SIGMA_LOW":
                setattr(self, key, lambda x, p=config[key]: self.Sigma_low(x, *p))
            else:
                setattr(self, key, config[key])

        # Confirm all attributes are set. 
        expected_keys = ["WORDSIZE","BLOCKSIZE","ITERATIONS","H0","K",
        "SIGMA_CAP_0","SIGMA_CAP_1","SIGMA_LOW_0","SIGMA_LOW_1",
        "W_SUMS","P_SIGMA_LOW_0","P_SIGMA_LOW_1","DIGEST_BITS","MODULE_NAME"]
        for key in expected_keys:
            getattr(self, key)

if __name__ == "__main__":
    import hashlib

    #Some static test cases
    data = b"qwertyuiopasdfghjklzxcvbnm1234567890QWERTYUIOPASDFGHJKLZXCVBNM"*77

    print("SHA224:")
    print(SHAAA(basetype="SHA224").digest(data).hex())
    sha224 = hashlib.sha224()
    sha224.update(data)
    print(sha224.hexdigest())


    print("SHA256:")
    print(SHAAA(basetype="SHA256").digest(data).hex())
    sha256 = hashlib.sha256()
    sha256.update(data)
    print(sha256.hexdigest())


    print("SHA384:")
    print(SHAAA(basetype="SHA384").digest(data).hex())
    sha384 = hashlib.sha384()
    sha384.update(data)
    print(sha384.hexdigest())


    print("SHA512:")
    print(SHAAA(basetype="SHA512").digest(data).hex())
    sha512 = hashlib.sha512()
    sha512.update(data)
    print(sha512.hexdigest())