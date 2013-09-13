# -*- coding: gbk -*- 

'''
conf module
@authors  :   U{raymondxie<mailto: raymondxie@tencent.com>}
@copyright:   tencent
@date     :   2010-07-14
@version  :   1.0.0.0
'''

import os, time,StringIO

from com.exception import ComException
from com.conf.configobj import ConfigObj

class ConfException(ComException):
    pass;
    
class Conf(object):
    def __init__(self, fn='',sep=':',comment='#'):
        '''
        @param fn:�ļ���
        @type fn: str    
        @param sep:key-value�ָ����������ǵ����ַ���Ҳ������(a|b)����ϣ�a��bΪ2�����ַ��ָ�,Ĭ��Ϊ:
        @type sep: str   
        @param comment:ע�ͣ������ǵ����ַ���Ҳ������(a|b)����ϣ�a��bΪ2�����ַ�ע�ͣ�Ĭ��Ϊ#
        @type comment: str           
        @note: [example] \n
            mc = Conf('test.conf','=',';') \n
            mc = Conf('test.conf','(=|:)','(#|;|//') \n
        '''      
        self.fn = fn;
        self.sep = sep;
        self.comment = comment;

        self.getValue = '';

        if isinstance(fn,StringIO.StringIO):
            self.fn = fn
        elif not os.path.exists( os.path.abspath((fn)) ):
            print ConfException.getExceptionStack()
            raise ConfException ,"file:%s is not existed" %(fn,)

        self.config = ConfigObj(self.fn,sep=sep,comment=comment)
          
    def add(self, key , value):
        '''
        @param key:���������
        @type key: str    
        @param value:������ֵ
        @type value: str   
        @note: [example] ����key - value �ԣ���sep�ָ�\n
            mc = Conf('test.conf','=',';') \n
            mc.add('Item1','123')\n
        '''          
        self.set(key,value);

    def delete(self, key):
        '''
        @param key:���������
        @type key: str      
        @note: [example] ɾ��key ��Ӧ�� value \n
            mc = Conf('test.conf','=',';') \n
            mc.delete('Item1')\n
        '''     
        try:    
            del self.config[key]
            self.config.write();
        except:
            print ConfException.getExceptionStack()
            pass

    def get(self, key):
        '''
        @param key:���������
        @type key: str    
        @return: ����������ֵ
        @rtype: str        
        @note: [example] ���key ��Ӧ�� value \n
            mc = Conf('test.conf','=',';') \n
            val = mc.get('Item1')\n
        '''      
        try:
            return str(self.config[key]);
        except:
            print ConfException.getExceptionStack()
            raise ConfException ,"key:%s isn't existed in file:%s" %(key,self.fn)

    def set(self, key, value):
        '''
        @param key:���������
        @type key: str    
        @param value:������ֵ
        @type value: str   
        @note: [example] �޸Ļ�����key - value �ԣ���sep�ָ�\n
            mc = Conf('test.conf','=',';') \n
            mc.set('Item1','123')\n
        '''      
        try:
            self.config[key] = value;
            self.config.write();
        except:
            pass

    def setBatch(self, kvDict):
        '''
        @param kvDict:���������<-->������ֵ �ʵ�
        @type key: dict    
        @note: [example] �����޸Ļ�����key - value �ԣ���sep�ָ�\n
            mc = Conf('test.conf','=',';') \n
            mc.setBatch({'Item1':'123','Item2':'456'})\n
        '''  
        try:
	        for k,v in kvDict.items():
	            self.config[k] = v;

	        self.config.write();
        except:
            pass
        
    def hasKey(self, key):
        '''
        @param key:���������
        @type key: str  
        @return: key���ڣ�����True; ���򷵻�False
        @rtype: bool            
        @note: [example] �ж�key �Ƿ����\n
            mc = Conf('test.conf','=',';') \n
            mc.hasKey('Item1') \n
        '''      
        try:
            self.config[key];
            return True;
        except:
            return False;
        
    def clear(self):
        '''
        @note: [example] �������\n
            mc = Conf('test.conf','=',';') \n
            mc.clear()
        '''      
        fileid=open(self.fn, "w");
        fileid.writelines("");
        fileid.close();
        
class TextConf(Conf):
    pass;
    
class IniConf(Conf):

    def add(self, key , value,*section):
        '''
        @param key:���������
        @type key: str    
        @param value:������ֵ
        @type value: str 
        @param section:ָ��key��Ӧ��section
        @type section: tuple           
        @note: [example] ����key - value �ԣ���sep�ָ�\n
            mc = IniConf('test.conf','=',';') \n
            mc.add('Item1','123') #��ȫ������Item1\n 
            mc.add('Item1','123','Section1')#��[Section1] ����Item1\n
            mc.add('Item1','123','Section1','Section11')#��[Section1[Section11]] ����Item1\n
        '''      
        self.set(key,value,section);

    def delete(self, key,*section):
        '''
        @param key:���������
        @type key: str  
        @param section:ָ��key��Ӧ��section
        @type section: tuple           
        @note: [example] ɾ��key ��Ӧ�� value \n
            mc = IniConf('test.conf','=',';') \n
            mc.delete('Item1') #ɾ��ȫ�ֵ�Item1\n 
            mc.delete('Item1','Section1')#ɾ��[Section1]��Item1\n
            mc.delete('Item1','Section1','Section11')#ɾ��[Section1[Section11]] ��Item1\n
        '''     
        if section  == ():
            return Conf.delete(self,key);
            
        try:    
            h = self.config;
            for i in range(0,len(section)):
                h=h[section[i]]
                
            del h[key]
            self.config.write();
        except:
            pass

    def get(self, key,*section):
        '''
        @param key:���������
        @type key: str  
        @param section:ָ��key��Ӧ��section
        @type section: tuple           
        @return: ����������ֵ
        @rtype: str        
        @note: [example] ���key ��Ӧ�� value \n
            mc = IniConf('test.conf','=',';') \n
            val =mc.get('Item1') #���ȫ�ֵ�Item1\n 
            val =mc.get('Item1','Section1')#���[Section1]��Item1\n
            val =mc.get('Item1','Section1','Section11')#���[Section1[Section11]] ��Item1\n
        '''      
        try:
            if section == ():
                return str(Conf.get(self,key))
	            
            h = self.config;
            for i in range(0,len(section)):
                h=h[section[i]]
              
            return str(h[key])
        except:
            print ConfException.getExceptionStack()
            raise ConfException ,"key:%s isn't existed in file:%s section:%s" %(key,self.fn,section)

    def set(self, key, value,*section):
        '''
        @param key:���������
        @type key: str    
        @param value:������ֵ
        @type value: str   
        @param section:ָ��key��Ӧ��section
        @type section: tuple           
        @note: [example] �޸Ļ�����key - value �ԣ���sep�ָ�\n
            mc = IniConf('test.conf','=',';') \n
            mc.set('Item1','123') #��ȫ���޸Ļ�����Item1\n 
            mc.set('Item1','123','Section1')#��[Section1] �޸Ļ�����Item1\n
            mc.set('Item1','123','Section1','Section11')#��[Section1[Section11]] �޸Ļ�����Item1\n
        '''      
        try:
            key=str(key)
            value = str(value)
            if section == ():
                self.config[key] = value;
                self.config.write();
                return ;   

            h = self.config;
            for i in range(0,len(section)):
                h=h[section[i]]
                
            h[key] = value;
            self.config.write();
        except:
            print ConfException.getExceptionStack()
            raise ConfException ,"set key:%s,value:%s,setction:%s in file:%s error" %(key,value,section,self.fn)

    def setBatch(self, skvDict):
        '''
        @param skvDict: ��ʽΪ{tuple:dict}
        @type skvDict: dict    
        @note: [example] �����޸Ļ�����key - value �ԣ���sep�ָ�\n
            mc = IniConf('test.conf','=',';') \n
            kvd = {(s1,s2):{k1:v1,k2:v2}}\n
            mc.setBatch(kvd)\n
            ==>\n
            self.config[s1][s2][k1] = v1\n
            self.config[s1][s2][k2] = v2   
        '''     
        try:
            for s,kv in skvDict.items():
                h = self.config;
                if isinstance(s,str):
                    s = (s,)
                for i in range(0,len(s)):
                  h=h[s[i]]
                
                for k,v in kv.items():
                    h[k] = v;
        
            self.config.write();
        except:
            print ConfException.getExceptionStack()
            raise ConfException ,'''set skvDict:%s in file:%s error.\n 
                                    skvDict format:{tuple:dict}
                                    example:
                                        {(s1,s2):{k1:v1,k2:v2}}\n
                                    that is to set:\n
									                      self.config[s1][s2][k1] = v1\n
						 			                      self.config[s1][s2][k2] = v2\n
 			                           '''  %(skvDict,self.fn)
    
    def hasKey(self, key,*section):
        '''
        @param key:���������
        @type key: str  
        @param section:ָ��key��Ӧ��section
        @type section: tuple          
        @return: key���ڣ�����True; ���򷵻�False
        @rtype: bool            
        @note: [example] �ж�key �Ƿ����\n
            mc = IniConf('test.conf','=',';') \n
            mc.hasKey('Item1','Section1') #[Section1]���Ƿ����Item1\n
            mc.hasKey('Item1','Section1','Section11') #[Section1[Section11]]���Ƿ����Item1\n
        '''   
        try:
            if section == ():
                self.config[key];
                return True;
	            
            h = self.config;
            for i in range(0,len(section)):
                h=h[section[i]]        
            h[key];
            return True;
        except:
            return False;