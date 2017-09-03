import queue
import serial
from threading import Thread
   
class Arduino(object):
    def __init__(self):
        self.inputQ = queue.Queue()
        self.outputQ = queue.Queue()
        self.serialPort=None
        self.inputThread = None
        self.outputThread = None
        self.keepRunning = True
        self.activeConnection = False
        self.ID_string = 'none'

    def connect(self,portName):
        print("Trying to open "+portName)
        if self.activeConnection:     
            self.close()
            self.activeConnection = False
        try:
            # self.serialPort = serial.Serial("COM5",9600)
            # self.serialPort = serial.Serial(portName,28800)
            self.serialPort = serial.Serial(portName,115200)
        except serial.SerialException:
            self.inputQ.put("ERROR: No connection on "+portName)
            return
        self.keepRunning=True
        self.inputThread = Thread(target=self.runInput)
        self.inputThread.daemon=True
        self.inputThread.start()
        self.outputThread = Thread(target=self.runOutput)
        self.outputThread.daemon=True
        self.outputThread.start()
        self.activeConnection = True
        self.inputQ.put("Connected!!")
        tempString = str(self.serialPort)
        self.ID_string = tempString[tempString.find('<')+1:tempString.find(',')]
        print(self.ID_string)

    def getInput(self):
        
        """
        None -> [int, char, int, int, int]
        boxNum = 99
        charCode = "XX"
        timeStamp = 0
        level = 0
        boolVarListIndex = -1                       

        tokens = inputLine.split()
        if len(tokens) > 5:         # this prints the report
            print(inputLine)            
        try:
            if len(tokens) > 2:
                boxNum = (int(tokens[0].decode()))
                charCode = tokens[1]      # eg b"L"
                charCode = charCode.decode()   # convert from "byte literal"
                timeStamp = int(tokens[2].decode())
            if len(tokens) > 3:
                level = (int(tokens[3].decode()))
                boolVarListIndex = (int(tokens[4].decode()))
        except:
            print('exception at getInput: ',tokens)
        tokenList = [boxNum,charCode, timeStamp,level,boolVarListIndex]
        """
        inputLine = self.inputQ.get_nowait()
        return inputLine
        

    def runInput(self):
        while self.keepRunning:
            try:
                inputline = self.serialPort.readline()
                self.inputQ.put(inputline.rstrip())
            except:
                # This section reached when the connection closes
                pass
        return

    def runOutput(self):
        while self.keepRunning:
            try:
                outputline = self.outputQ.get()
                self.serialPort.write(bytes(outputline.encode('ascii')))
            except:
                # This section reached when the connection closes
                pass
        return


    def close(self):
        self.keepRunning=False;
        self.serialPort.close()
        self.outputQ.put("TERMINATE") # This does not get sent, but stops the outputQ from blocking.
        self.inputThread.join()
        self.outputThread.join()
        self.inputQ.put("All I/O Threads stopped")
