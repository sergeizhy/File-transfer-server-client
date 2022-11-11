from base64 import b64decode,b64encode
from zlib import crc32
import binascii 
import uuid
import os

__all__ = ['Stream']

class Stream():
    
    @staticmethod
    def crc32(bytes):
        return int(crc32(bytes))
        
    @staticmethod
    def generate_uuid(value,seed = uuid.NAMESPACE_X500):
        return uuid.uuid3(seed,value)

    @staticmethod
    def hexlify(data,sep = ' ',bytes_per_sep = 0,encoding = 'ascii'):
        hex = binascii.hexlify(data,sep,bytes_per_sep)
        if encoding is None:
            return hex
        return str(hex,encoding)

    @staticmethod
    def encoed_base64(bytes,encoding = 'ascii'):
        b64 = b64encode(bytes)
        if encoding is None :
            return b64
        return str(b64,encoding)
  
    @staticmethod
    def decode_base64(bytes,encoding = 'ascii'):
        b64 = b64decode(bytes)
        if encoding is None:
            return b64
        return str(b64,encoding)
    
    @staticmethod
    def read_from_file(file, mod = 'r'):
        data = []
        try:
            with open(file,mod) as fp:
                data = fp.readlines()
        except IOError as error:
            print(error)
        return data
        
    @staticmethod
    def write_to_file(file,data,mod = 'w'):
        try:
            with open(file,mod) as fp:
                fp.write(data)
        except IOError as error:
            print(error)
    @staticmethod
    def stringify(data,encoding ='ascii'):
        s = str(data,encoding)
        return s.strip('\x00').strip()

    @staticmethod
    def create_folder(directory):
        try:
            parent = os.getcwd()
            path = os.path.join(parent,directory)
            if not os.path.exists(path):
                os.mkdir(path)
            return path
        except OSError as error:
            print(error)
