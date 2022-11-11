from Crypto.Cipher import AES,PKCS1_OAEP
from Crypto.PublicKey import RSA
from Crypto.Util.Padding import unpad
from Crypto.Random import get_random_bytes

__all__ = ['Symmetric','Asymmetric']

BLOCK_SIZE = 16
AES_KEY = 16
RSA_KEY = 128
 
class Symmetric:
    
    def __init__(self):
        self.key = get_random_bytes(BLOCK_SIZE)
        self.iv = b'\x00' * BLOCK_SIZE

    def decrypt(self,raw) -> bytes:
        cipher = AES.new(self.key,AES.MODE_CBC,iv=self.iv)
        data = cipher.decrypt(raw)
        data = unpad(data,BLOCK_SIZE)
        return data
    
    def load(self,key) -> None:
        if len(key) == AES_KEY :
            self.key = key
        
class Asymmetric:
    def __init__(self):
        self.public_key = None
    
    def encrypt(self,data) -> bytes:
        oaep = PKCS1_OAEP.new(self.public_key)
        cipher = oaep.encrypt(data)
        return cipher

    def load(self,key) -> None:
        public_key =                        \
            '-----BEGIN PUBLIC KEY-----\n'  \
            + key +                         \
            '\n-----END PUBLIC KEY-----'

        self.public_key = RSA.import_key(public_key)