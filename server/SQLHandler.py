import sqlite3
import logging
import threading
from datetime import datetime

__all__ = ['SQL']

class SQL:
    secondry = 0
    primary = 1
    connected = False
    database = ['db/server.db',':memory:']
    queryQueue = []
    thread = None
    
    def __init__(self):
        self.data = []
        self.back = False
    
    def insert_user(self, uuid,name):
        query = '''insert into clients(ID, Name, PublicKey, LastSeen, AESKey)
                     values(?, ?, 0, 0, 0);'''
        args = (None,query, (uuid,name))
        SQL.queryQueue.append(args)
        self.update_time(uuid)

    def insert_file(self,uuid,file_name,path_name,verified):
        query = '''insert into files(ID, "File Name", "Path Name", Verified)      
                     values(?, ?, ?, ?);'''

        args = (None,query,(uuid,file_name,path_name,verified))
        SQL.queryQueue.append(args)

    def search_user(self,uuid,name):
        self.data = []
        self.back = False
        query = 'select * from clients where ID = ? and Name = ?;'
        args = (self,query,(uuid,name))
        SQL.queryQueue.append(args)
        while not self.back:
            pass
        return self.data

    def update_keys(self,public_key,aes_key,name,uuid):
        query = "update clients set (PublicKey,AESKey) = (?,?) where Name = ?"
        args = (None,query,(public_key,aes_key,name))
        SQL.queryQueue.append(args)
        self.update_time(uuid)
    
    def update_time(self,uuid):
        curr_time = str(datetime.now())
        query = "update clients set LastSeen = ? where ID = ?"
        args = (None,query,(curr_time,uuid))
        SQL.queryQueue.append(args)

    def verified(self,file_name):
        query = 'update files set Verified = ? where "File Name" = ?'
        args = (None,query,(1,file_name,))
        SQL.queryQueue.append(args)
        
    @staticmethod
    def worker():
        connections : list = []
        for db in SQL.database:
            connections.append(sqlite3.connect(db))
        SQL.load(connections[SQL.secondry],connections[SQL.primary])

        while SQL.connected:
            active : sqlite3.Connection = connections[SQL.primary]
            while len(SQL.queryQueue):
                obj,query,param = SQL.queryQueue.pop(0)
                try:
                    cursor = active.cursor()
                    data = cursor.execute(query,param)
                    if obj is not None:
                        obj.data = data.fetchall()
                        obj.back = True
                    active.commit()
                    cursor.close()
                    SQL.load(connections[SQL.primary],connections[SQL.secondry],2)
                except sqlite3.Error as error:
                    print(error)
        
        for connection in connections:
            connection.close()

    @staticmethod
    def load(src : sqlite3.Connection, dst : sqlite3.Connection,p = 0):
        src.backup(dst,pages=p)

    @staticmethod
    def open_connection():
        try:
            if not SQL.connected:
                SQL.connected = True
                SQL.thread = threading.Thread(target=SQL.worker)
                SQL.init()
                SQL.thread.start()

        except OSError as error:
            print(error)
    
    @staticmethod
    def init():
        queries = [
            '''create table if not exists clients(
                ID text primary key not null,   
                Name text not null,             
                PublicKey text not null,        
                LastSeen text not null,         
                AESKey text not null);'''
                ,
            '''create table if not exists files (
                ID text not null,   
                "File Name" text not null,      
                "Path Name" text not null,      
                Verified integer not null);'''
            ]
            
        for query in queries:
            SQL.queryQueue.append((None,query,()))

    @staticmethod
    def close_connection():
        if SQL.connected:
            SQL.connected = False

    
    