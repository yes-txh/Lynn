#
# Autogenerated by Thrift Compiler (0.9.0)
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#
#  options string: py
#

from thrift.Thrift import TType, TMessageType, TException, TApplicationException

from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol, TProtocol
try:
  from thrift.protocol import fastbinary
except:
  fastbinary = None


class TaskEntityState:
  TASKENTITY_NOTFOUND = 0
  TASKENTITY_WAITING = 1
  TASKENTITY_STARTING = 2
  TASKENTITY_STARTED = 3
  TASKENTITY_STOPED = 4
  TASKENTITY_FINISHED = 5
  TASKENTITY_FAILED = 6

  _VALUES_TO_NAMES = {
    0: "TASKENTITY_NOTFOUND",
    1: "TASKENTITY_WAITING",
    2: "TASKENTITY_STARTING",
    3: "TASKENTITY_STARTED",
    4: "TASKENTITY_STOPED",
    5: "TASKENTITY_FINISHED",
    6: "TASKENTITY_FAILED",
  }

  _NAMES_TO_VALUES = {
    "TASKENTITY_NOTFOUND": 0,
    "TASKENTITY_WAITING": 1,
    "TASKENTITY_STARTING": 2,
    "TASKENTITY_STARTED": 3,
    "TASKENTITY_STOPED": 4,
    "TASKENTITY_FINISHED": 5,
    "TASKENTITY_FAILED": 6,
  }

class VMType:
  VM_UNKNOWN = 0
  VM_KVM = 1
  VM_LXC = 2

  _VALUES_TO_NAMES = {
    0: "VM_UNKNOWN",
    1: "VM_KVM",
    2: "VM_LXC",
  }

  _NAMES_TO_VALUES = {
    "VM_UNKNOWN": 0,
    "VM_KVM": 1,
    "VM_LXC": 2,
  }

class VMState:
  VM_NOTFOUND = 0
  VM_OFFLINE = 1
  VM_ONLINE = 2
  VM_SERVICE_ONLINE = 3

  _VALUES_TO_NAMES = {
    0: "VM_NOTFOUND",
    1: "VM_OFFLINE",
    2: "VM_ONLINE",
    3: "VM_SERVICE_ONLINE",
  }

  _NAMES_TO_VALUES = {
    "VM_NOTFOUND": 0,
    "VM_OFFLINE": 1,
    "VM_ONLINE": 2,
    "VM_SERVICE_ONLINE": 3,
  }

class AppState:
  APP_NOTFOUND = 0
  APP_ONLINE = 1
  APP_FINISHED = 2
  APP_FAILED = 3

  _VALUES_TO_NAMES = {
    0: "APP_NOTFOUND",
    1: "APP_ONLINE",
    2: "APP_FINISHED",
    3: "APP_FAILED",
  }

  _NAMES_TO_VALUES = {
    "APP_NOTFOUND": 0,
    "APP_ONLINE": 1,
    "APP_FINISHED": 2,
    "APP_FAILED": 3,
  }

class EventType:
  STATE_EVENT = 0
  ACTION_EVENT = 1
  TASK_EVENT = 2

  _VALUES_TO_NAMES = {
    0: "STATE_EVENT",
    1: "ACTION_EVENT",
    2: "TASK_EVENT",
  }

  _NAMES_TO_VALUES = {
    "STATE_EVENT": 0,
    "ACTION_EVENT": 1,
    "TASK_EVENT": 2,
  }


class VM_AppInfo:
  """
  Attributes:
   - id
   - name
   - app_source
   - install_dir
   - exe
   - argument
   - out_dir
   - app_out_dir
   - run_type
   - interval
  """

  thrift_spec = (
    None, # 0
    (1, TType.I32, 'id', None, None, ), # 1
    (2, TType.STRING, 'name', None, None, ), # 2
    (3, TType.STRING, 'app_source', None, None, ), # 3
    (4, TType.STRING, 'install_dir', None, None, ), # 4
    (5, TType.STRING, 'exe', None, None, ), # 5
    (6, TType.STRING, 'argument', None, None, ), # 6
    (7, TType.STRING, 'out_dir', None, None, ), # 7
    (8, TType.STRING, 'app_out_dir', None, None, ), # 8
    (9, TType.STRING, 'run_type', None, None, ), # 9
    (10, TType.I32, 'interval', None, None, ), # 10
  )

  def __init__(self, id=None, name=None, app_source=None, install_dir=None, exe=None, argument=None, out_dir=None, app_out_dir=None, run_type=None, interval=None,):
    self.id = id
    self.name = name
    self.app_source = app_source
    self.install_dir = install_dir
    self.exe = exe
    self.argument = argument
    self.out_dir = out_dir
    self.app_out_dir = app_out_dir
    self.run_type = run_type
    self.interval = interval

  def read(self, iprot):
    if iprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastbinary is not None:
      fastbinary.decode_binary(self, iprot.trans, (self.__class__, self.thrift_spec))
      return
    iprot.readStructBegin()
    while True:
      (fname, ftype, fid) = iprot.readFieldBegin()
      if ftype == TType.STOP:
        break
      if fid == 1:
        if ftype == TType.I32:
          self.id = iprot.readI32();
        else:
          iprot.skip(ftype)
      elif fid == 2:
        if ftype == TType.STRING:
          self.name = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 3:
        if ftype == TType.STRING:
          self.app_source = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 4:
        if ftype == TType.STRING:
          self.install_dir = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 5:
        if ftype == TType.STRING:
          self.exe = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 6:
        if ftype == TType.STRING:
          self.argument = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 7:
        if ftype == TType.STRING:
          self.out_dir = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 8:
        if ftype == TType.STRING:
          self.app_out_dir = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 9:
        if ftype == TType.STRING:
          self.run_type = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 10:
        if ftype == TType.I32:
          self.interval = iprot.readI32();
        else:
          iprot.skip(ftype)
      else:
        iprot.skip(ftype)
      iprot.readFieldEnd()
    iprot.readStructEnd()

  def write(self, oprot):
    if oprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and self.thrift_spec is not None and fastbinary is not None:
      oprot.trans.write(fastbinary.encode_binary(self, (self.__class__, self.thrift_spec)))
      return
    oprot.writeStructBegin('VM_AppInfo')
    if self.id is not None:
      oprot.writeFieldBegin('id', TType.I32, 1)
      oprot.writeI32(self.id)
      oprot.writeFieldEnd()
    if self.name is not None:
      oprot.writeFieldBegin('name', TType.STRING, 2)
      oprot.writeString(self.name)
      oprot.writeFieldEnd()
    if self.app_source is not None:
      oprot.writeFieldBegin('app_source', TType.STRING, 3)
      oprot.writeString(self.app_source)
      oprot.writeFieldEnd()
    if self.install_dir is not None:
      oprot.writeFieldBegin('install_dir', TType.STRING, 4)
      oprot.writeString(self.install_dir)
      oprot.writeFieldEnd()
    if self.exe is not None:
      oprot.writeFieldBegin('exe', TType.STRING, 5)
      oprot.writeString(self.exe)
      oprot.writeFieldEnd()
    if self.argument is not None:
      oprot.writeFieldBegin('argument', TType.STRING, 6)
      oprot.writeString(self.argument)
      oprot.writeFieldEnd()
    if self.out_dir is not None:
      oprot.writeFieldBegin('out_dir', TType.STRING, 7)
      oprot.writeString(self.out_dir)
      oprot.writeFieldEnd()
    if self.app_out_dir is not None:
      oprot.writeFieldBegin('app_out_dir', TType.STRING, 8)
      oprot.writeString(self.app_out_dir)
      oprot.writeFieldEnd()
    if self.run_type is not None:
      oprot.writeFieldBegin('run_type', TType.STRING, 9)
      oprot.writeString(self.run_type)
      oprot.writeFieldEnd()
    if self.interval is not None:
      oprot.writeFieldBegin('interval', TType.I32, 10)
      oprot.writeI32(self.interval)
      oprot.writeFieldEnd()
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def validate(self):
    return


  def __repr__(self):
    L = ['%s=%r' % (key, value)
      for key, value in self.__dict__.iteritems()]
    return '%s(%s)' % (self.__class__.__name__, ', '.join(L))

  def __eq__(self, other):
    return isinstance(other, self.__class__) and self.__dict__ == other.__dict__

  def __ne__(self, other):
    return not (self == other)

class VM_HbAppInfo:
  """
  Attributes:
   - id
   - name
   - state
   - error_id
  """

  thrift_spec = (
    None, # 0
    (1, TType.I32, 'id', None, None, ), # 1
    (2, TType.STRING, 'name', None, None, ), # 2
    (3, TType.I32, 'state', None, None, ), # 3
    (4, TType.I32, 'error_id', None, None, ), # 4
  )

  def __init__(self, id=None, name=None, state=None, error_id=None,):
    self.id = id
    self.name = name
    self.state = state
    self.error_id = error_id

  def read(self, iprot):
    if iprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastbinary is not None:
      fastbinary.decode_binary(self, iprot.trans, (self.__class__, self.thrift_spec))
      return
    iprot.readStructBegin()
    while True:
      (fname, ftype, fid) = iprot.readFieldBegin()
      if ftype == TType.STOP:
        break
      if fid == 1:
        if ftype == TType.I32:
          self.id = iprot.readI32();
        else:
          iprot.skip(ftype)
      elif fid == 2:
        if ftype == TType.STRING:
          self.name = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 3:
        if ftype == TType.I32:
          self.state = iprot.readI32();
        else:
          iprot.skip(ftype)
      elif fid == 4:
        if ftype == TType.I32:
          self.error_id = iprot.readI32();
        else:
          iprot.skip(ftype)
      else:
        iprot.skip(ftype)
      iprot.readFieldEnd()
    iprot.readStructEnd()

  def write(self, oprot):
    if oprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and self.thrift_spec is not None and fastbinary is not None:
      oprot.trans.write(fastbinary.encode_binary(self, (self.__class__, self.thrift_spec)))
      return
    oprot.writeStructBegin('VM_HbAppInfo')
    if self.id is not None:
      oprot.writeFieldBegin('id', TType.I32, 1)
      oprot.writeI32(self.id)
      oprot.writeFieldEnd()
    if self.name is not None:
      oprot.writeFieldBegin('name', TType.STRING, 2)
      oprot.writeString(self.name)
      oprot.writeFieldEnd()
    if self.state is not None:
      oprot.writeFieldBegin('state', TType.I32, 3)
      oprot.writeI32(self.state)
      oprot.writeFieldEnd()
    if self.error_id is not None:
      oprot.writeFieldBegin('error_id', TType.I32, 4)
      oprot.writeI32(self.error_id)
      oprot.writeFieldEnd()
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def validate(self):
    return


  def __repr__(self):
    L = ['%s=%r' % (key, value)
      for key, value in self.__dict__.iteritems()]
    return '%s(%s)' % (self.__class__.__name__, ', '.join(L))

  def __eq__(self, other):
    return isinstance(other, self.__class__) and self.__dict__ == other.__dict__

  def __ne__(self, other):
    return not (self == other)

class VM_HbVMInfo:
  """
  Attributes:
   - job_id
   - task_id
   - cpu_usage
   - memory_usage
   - bytes_in
   - bytes_out
   - state
   - app_running
   - hb_app_info
  """

  thrift_spec = (
    None, # 0
    (1, TType.I32, 'job_id', None, None, ), # 1
    (2, TType.I32, 'task_id', None, None, ), # 2
    (3, TType.DOUBLE, 'cpu_usage', None, None, ), # 3
    (4, TType.DOUBLE, 'memory_usage', None, None, ), # 4
    (5, TType.I32, 'bytes_in', None, None, ), # 5
    (6, TType.I32, 'bytes_out', None, None, ), # 6
    (7, TType.I32, 'state', None, None, ), # 7
    (8, TType.BOOL, 'app_running', None, None, ), # 8
    (9, TType.STRUCT, 'hb_app_info', (VM_HbAppInfo, VM_HbAppInfo.thrift_spec), None, ), # 9
  )

  def __init__(self, job_id=None, task_id=None, cpu_usage=None, memory_usage=None, bytes_in=None, bytes_out=None, state=None, app_running=None, hb_app_info=None,):
    self.job_id = job_id
    self.task_id = task_id
    self.cpu_usage = cpu_usage
    self.memory_usage = memory_usage
    self.bytes_in = bytes_in
    self.bytes_out = bytes_out
    self.state = state
    self.app_running = app_running
    self.hb_app_info = hb_app_info

  def read(self, iprot):
    if iprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastbinary is not None:
      fastbinary.decode_binary(self, iprot.trans, (self.__class__, self.thrift_spec))
      return
    iprot.readStructBegin()
    while True:
      (fname, ftype, fid) = iprot.readFieldBegin()
      if ftype == TType.STOP:
        break
      if fid == 1:
        if ftype == TType.I32:
          self.job_id = iprot.readI32();
        else:
          iprot.skip(ftype)
      elif fid == 2:
        if ftype == TType.I32:
          self.task_id = iprot.readI32();
        else:
          iprot.skip(ftype)
      elif fid == 3:
        if ftype == TType.DOUBLE:
          self.cpu_usage = iprot.readDouble();
        else:
          iprot.skip(ftype)
      elif fid == 4:
        if ftype == TType.DOUBLE:
          self.memory_usage = iprot.readDouble();
        else:
          iprot.skip(ftype)
      elif fid == 5:
        if ftype == TType.I32:
          self.bytes_in = iprot.readI32();
        else:
          iprot.skip(ftype)
      elif fid == 6:
        if ftype == TType.I32:
          self.bytes_out = iprot.readI32();
        else:
          iprot.skip(ftype)
      elif fid == 7:
        if ftype == TType.I32:
          self.state = iprot.readI32();
        else:
          iprot.skip(ftype)
      elif fid == 8:
        if ftype == TType.BOOL:
          self.app_running = iprot.readBool();
        else:
          iprot.skip(ftype)
      elif fid == 9:
        if ftype == TType.STRUCT:
          self.hb_app_info = VM_HbAppInfo()
          self.hb_app_info.read(iprot)
        else:
          iprot.skip(ftype)
      else:
        iprot.skip(ftype)
      iprot.readFieldEnd()
    iprot.readStructEnd()

  def write(self, oprot):
    if oprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and self.thrift_spec is not None and fastbinary is not None:
      oprot.trans.write(fastbinary.encode_binary(self, (self.__class__, self.thrift_spec)))
      return
    oprot.writeStructBegin('VM_HbVMInfo')
    if self.job_id is not None:
      oprot.writeFieldBegin('job_id', TType.I32, 1)
      oprot.writeI32(self.job_id)
      oprot.writeFieldEnd()
    if self.task_id is not None:
      oprot.writeFieldBegin('task_id', TType.I32, 2)
      oprot.writeI32(self.task_id)
      oprot.writeFieldEnd()
    if self.cpu_usage is not None:
      oprot.writeFieldBegin('cpu_usage', TType.DOUBLE, 3)
      oprot.writeDouble(self.cpu_usage)
      oprot.writeFieldEnd()
    if self.memory_usage is not None:
      oprot.writeFieldBegin('memory_usage', TType.DOUBLE, 4)
      oprot.writeDouble(self.memory_usage)
      oprot.writeFieldEnd()
    if self.bytes_in is not None:
      oprot.writeFieldBegin('bytes_in', TType.I32, 5)
      oprot.writeI32(self.bytes_in)
      oprot.writeFieldEnd()
    if self.bytes_out is not None:
      oprot.writeFieldBegin('bytes_out', TType.I32, 6)
      oprot.writeI32(self.bytes_out)
      oprot.writeFieldEnd()
    if self.state is not None:
      oprot.writeFieldBegin('state', TType.I32, 7)
      oprot.writeI32(self.state)
      oprot.writeFieldEnd()
    if self.app_running is not None:
      oprot.writeFieldBegin('app_running', TType.BOOL, 8)
      oprot.writeBool(self.app_running)
      oprot.writeFieldEnd()
    if self.hb_app_info is not None:
      oprot.writeFieldBegin('hb_app_info', TType.STRUCT, 9)
      self.hb_app_info.write(oprot)
      oprot.writeFieldEnd()
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def validate(self):
    return


  def __repr__(self):
    L = ['%s=%r' % (key, value)
      for key, value in self.__dict__.iteritems()]
    return '%s(%s)' % (self.__class__.__name__, ', '.join(L))

  def __eq__(self, other):
    return isinstance(other, self.__class__) and self.__dict__ == other.__dict__

  def __ne__(self, other):
    return not (self == other)
