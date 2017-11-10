"""
Master

Initial commit was from SelfAdmin202a.py
Renamed to SA200.py

"""


from tkinter import *
import tkinter.ttk as ttk
import tkinter.scrolledtext
import serial
import serial.tools.list_ports
import os
import glob
import time
import queue
from datetime import date
import ArdLib
import GraphLib
from time import sleep


def main(argv=None):
    if argv is None:
        argv = sys.argv
    gui = GuiClass()
    gui.go()
    return 0

class GuiClass(object):
    def __init__(self):

        self.version = "SA200.04"
        self.verbose = True
        self.sched = ['0: Do not run', '1: FR', '2:FR x 20', '3: FR x 40', '4: PR', '5: TH', '6: IntA: 5-25']
        self.box1 = Box(1)    # note that boxes[0] is box1
        self.box2 = Box(2)
        self.box3 = Box(3)
        self.box4 = Box(4)
        self.box5 = Box(5)
        self.box6 = Box(6)
        self.box7 = Box(7)
        self.box8 = Box(8)
        self.example1List = []
        self.example2List = []
        self.boxes = [self.box1,self.box2,self.box3,self.box4,self.box5,self.box6,self.box7,self.box8]
        # ********  Display stuff ************
        self.X_ZERO = 50
        self.Y_ZERO = 275
        self.CANVAS_WIDTH = 600
        self.CANVAS_HEIGHT = 300
        
        # *****************************
        self.arduino0 = ArdLib.Arduino()
        self.dataStreamOn = True
        self.echoCount = 0

        # ******************* tk - GUI interface   ****************************** 
        self.root = Tk()
        self.root.title(self.version)

        # ***** tk specific variables (StringVars, IntVars and BooleanVars) *****

        self.selectMax_x_Scale = IntVar(value = 180)
        self.selectedBox = IntVar(value = 0)
        self.OS_String = StringVar(value="OS = ?")

        # ***** Pump Chaos ****
        self.pumpChaosCheckVar = BooleanVar(value=False)
        self.pumpChaosBoxNum = 0
        self.pumpChaosState = False
        self.pumpChaosDelay = 3
        self.pumpChaosCount = 0
        # *********************
        
        self.showDataStreamCheckVar = BooleanVar(value=False)
        self.checkLeversCheckVar = BooleanVar(value=True)
        self.showOutputCheckVar = BooleanVar(value=False)
        self.portString = StringVar(value="----")             # will be assigned value from last line of ini file
        self.B1_lever1CheckVar = BooleanVar(value=False)      # or set it later this way: self.lever1CheckVar.set(False)
        self.B1_lever2CheckVar = BooleanVar(value=False) 
        self.B1_LED1CheckVar   = BooleanVar(value=False)
        self.B1_LED2CheckVar   = BooleanVar(value=False)
        self.B1_pumpCheckVar  = BooleanVar(value=False)
        self.B1_IDStr = StringVar(value="box1")                       
        self.B1_Weight = IntVar(value=100)
        self.B1_sched = StringVar(value="FR")         
        self.B1_FR_value = IntVar(value=1)         
        self.B1_SessionLength = IntVar(value=2)  # added Should it take a value or an index to the array in the Spinbox?
        self.B1_PumpTime = IntVar(value=4000)
        self.B1_calcPumpTime = BooleanVar(value=True)
        self.B1_L1Resp   = IntVar(value=0)
        self.B1_L2Resp   = IntVar(value=0)
        self.B1_Inf    = IntVar(value=0)
        self.B1_SessionTimeStr = StringVar(value="0:0:00")

        self.B2_lever1CheckVar = BooleanVar(value=False)
        self.B2_lever2CheckVar = BooleanVar(value=False) 
        self.B2_LED1CheckVar   = BooleanVar(value=False)
        self.B2_LED2CheckVar   = BooleanVar(value=False)
        self.B2_pumpCheckVar  = BooleanVar(value=False)
        self.B2_IDStr = StringVar(value="box2")
        self.B2_Weight = IntVar(value=100)
        self.B2_sched = StringVar(value="FR")             
        self.B2_FR_value = IntVar(value=1)        
        self.B2_SessionLength = IntVar(value=2)  
        self.B2_PumpTime = IntVar(value=4000)
        self.B2_calcPumpTime = BooleanVar(value=True)
        self.B2_L1Resp   = IntVar(value=0)
        self.B2_L2Resp   = IntVar(value=0)
        self.B2_Inf    = IntVar(value=0)
        self.B2_SessionTimeStr = StringVar(value="0:0:00")

        self.B3_lever1CheckVar = BooleanVar(value=False)
        self.B3_lever2CheckVar = BooleanVar(value=False) 
        self.B3_LED1CheckVar   = BooleanVar(value=False)
        self.B3_LED2CheckVar   = BooleanVar(value=False)
        self.B3_pumpCheckVar  = BooleanVar(value=False)
        self.B3_IDStr = StringVar(value="box3")
        self.B3_Weight = IntVar(value=100)
        self.B3_sched = StringVar(value="FR")            
        self.B3_FR_value = IntVar(value=1)         
        self.B3_SessionLength = IntVar(value=2)  
        self.B3_PumpTime = IntVar(value=4000)
        self.B3_calcPumpTime = BooleanVar(value=True)
        self.B3_L1Resp   = IntVar(value=0)
        self.B3_L2Resp   = IntVar(value=0)
        self.B3_Inf    = IntVar(value=0)
        self.B3_SessionTimeStr = StringVar(value="0:0:00")

        self.B4_lever1CheckVar = BooleanVar(value=False)
        self.B4_lever2CheckVar = BooleanVar(value=False) 
        self.B4_LED1CheckVar   = BooleanVar(value=False)
        self.B4_LED2CheckVar   = BooleanVar(value=False)
        self.B4_pumpCheckVar  = BooleanVar(value=False)
        self.B4_IDStr = StringVar(value="box4")
        self.B4_Weight = IntVar(value=100)
        self.B4_sched = StringVar(value="FR")            
        self.B4_FR_value = IntVar(value=1)   
        self.B4_SessionLength = IntVar(value=2)
        self.B4_PumpTime = IntVar(value=4000)
        self.B4_calcPumpTime = BooleanVar(value=True)
        self.B4_L1Resp   = IntVar(value=0)
        self.B4_L2Resp   = IntVar(value=0)
        self.B4_Inf    = IntVar(value=0)
        self.B4_SessionTimeStr = StringVar(value="0:0:00")

        self.B5_lever1CheckVar = BooleanVar(value=False)
        self.B5_lever2CheckVar = BooleanVar(value=False) 
        self.B5_LED1CheckVar   = BooleanVar(value=False)
        self.B5_LED2CheckVar   = BooleanVar(value=False)
        self.B5_pumpCheckVar  = BooleanVar(value=False)
        self.B5_IDStr = StringVar(value="box5")
        self.B5_Weight = IntVar(value=100)
        self.B5_sched = StringVar(value="FR")             
        self.B5_FR_value = IntVar(value=1)       
        self.B5_SessionLength = IntVar(value=2)  
        self.B5_PumpTime = IntVar(value=4000)
        self.B5_calcPumpTime = BooleanVar(value=True)
        self.B5_L1Resp   = IntVar(value=0)
        self.B5_L2Resp   = IntVar(value=0)
        self.B5_Inf    = IntVar(value=0)
        self.B5_SessionTimeStr = StringVar(value="0:0:00")

        self.B6_lever1CheckVar = BooleanVar(value=False)
        self.B6_lever2CheckVar = BooleanVar(value=False) 
        self.B6_LED1CheckVar   = BooleanVar(value=False)
        self.B6_LED2CheckVar   = BooleanVar(value=False)
        self.B6_pumpCheckVar  = BooleanVar(value=False)
        self.B6_IDStr = StringVar(value="box6")
        self.B6_Weight = IntVar(value=100)
        self.B6_sched = StringVar(value="FR")           
        self.B6_FR_value = IntVar(value=1)      
        self.B6_SessionLength = IntVar(value=2) 
        self.B6_PumpTime = IntVar(value=4000)
        self.B6_calcPumpTime = BooleanVar(value=True)
        self.B6_L1Resp   = IntVar(value=0)
        self.B6_L2Resp   = IntVar(value=0)
        self.B6_Inf    = IntVar(value=0)
        self.B6_SessionTimeStr = StringVar(value="0:0:00")

        self.B7_lever1CheckVar = BooleanVar(value=False)
        self.B7_lever2CheckVar = BooleanVar(value=False) 
        self.B7_LED1CheckVar   = BooleanVar(value=False)
        self.B7_LED2CheckVar   = BooleanVar(value=False)
        self.B7_pumpCheckVar  = BooleanVar(value=False)
        self.B7_IDStr = StringVar(value="box7")
        self.B7_Weight = IntVar(value=100)
        self.B7_sched = StringVar(value="FR")          
        self.B7_FR_value = IntVar(value=1)       
        self.B7_SessionLength = IntVar(value=2)  
        self.B7_PumpTime = IntVar(value=4000)
        self.B7_calcPumpTime = BooleanVar(value=True)
        self.B7_L1Resp   = IntVar(value=0)
        self.B7_L2Resp   = IntVar(value=0)
        self.B7_Inf    = IntVar(value=0)
        self.B7_SessionTimeStr = StringVar(value="0:0:00")

        self.B8_lever1CheckVar = BooleanVar(value=False)
        self.B8_lever2CheckVar = BooleanVar(value=False) 
        self.B8_LED1CheckVar   = BooleanVar(value=False)
        self.B8_LED2CheckVar   = BooleanVar(value=False)
        self.B8_pumpCheckVar  = BooleanVar(value=False)
        self.B8_IDStr = StringVar(value="box8")
        self.B8_Weight = IntVar(value=100)
        self.B8_sched = StringVar(value="FR")             
        self.B8_FR_value = IntVar(value=1)        
        self.B8_SessionLength = IntVar(value=2)  
        self.B8_PumpTime = IntVar(value=4000)
        self.B8_calcPumpTime = BooleanVar(value=True)
        self.B8_L1Resp   = IntVar(value=0)
        self.B8_L2Resp   = IntVar(value=0)
        self.B8_Inf    = IntVar(value=0)
        self.B8_SessionTimeStr = StringVar(value="0:0:00")
        

        # ********************* Menus ********************
        
        menubar = tkinter.Menu(self.root)
        filemenu = tkinter.Menu(menubar,tearoff=False)
        menubar.add_cascade(label="File",menu=filemenu)
        filemenu.add_command(label="Load from INI File",command = self.loadFromINIFile)
        filemenu.add_command(label="Save to INI File",command = self.writeToINIFile)
        filemenu.add_separator()
        filemenu.add_command(label="Save all Data",command = self.saveAllData)
        
        debugMenu = tkinter.Menu(menubar,tearoff=False)
        menubar.add_cascade(label="Debug",menu=debugMenu)
        debugMenu.add_command(label="Doesn't point to anything")
        debugMenu.add_command(label="Toggle Input Echo",command = self.toggleEchoInput)
        debugMenu.add_command(label="Report Feather M0 Status",command = self.reportPortStatus)
        cumRecMenu = tkinter.Menu(menubar,tearoff=False)
        menubar.add_cascade(label="Cum Rec",menu=cumRecMenu)        
        cumRecMenu.add_command(label="Place holder for X axis")
        cumRecMenu.add_command(label="Place holder for Y axis")
        
        self.root.config(menu=menubar)

        # ****************   ControlFrame - top Row ****************

        self.TwoLeverCheckVar = BooleanVar(value=False) 
                                                
        self.ControlFrame = ttk.Frame(self.root, borderwidth=3, relief="sunken")
        self.ControlFrame.grid(column = 0, row = 0, columnspan=4, sticky = (EW))
        
        label1 = ttk.Label(self.ControlFrame, text="Feather M0")
        label1.grid(column = 0, row = 0,pady=5, padx=5)       

        portEntry = ttk.Entry(self.ControlFrame, width=6,textvariable = self.portString)
        portEntry.grid(column = 1, row = 0)
        
        connectButton = ttk.Button(self.ControlFrame,text="Connect",\
                                        command = self.connect)
        connectButton.grid(column = 2, row = 0, pady=10, padx=10)

        self.connectLabelText = StringVar(value="Not Connected")

        connectLabel = ttk.Label(self.ControlFrame, textvariable = self.connectLabelText)
        connectLabel.grid(column = 3, row = 0,pady=5, padx=5)

        #label3 = ttk.Label(self.ControlFrame, text="_________________________")
        #label3.grid(column = 4, row = 0,pady=5, padx=5) 

        twoLeverCheckButton = Checkbutton(self.ControlFrame, text = "Two Lever", variable = self.TwoLeverCheckVar, \
             onvalue = True, offvalue = False)
        twoLeverCheckButton.grid(column = 5, row = 0, padx = 100)

        startAllBoxesButton = ttk.Button(self.ControlFrame,text="Start Boxes 1-8", \
                                             command = self.startAllBoxes)
        startAllBoxesButton.grid(column = 6, row = 0, padx = 10, sticky = (E))

        stopAllBoxesButton = ttk.Button(self.ControlFrame,text="Stop Boxes 1-8", \
                                            command = self.stopAllBoxes)
        stopAllBoxesButton.grid(column = 7, row = 0, padx =10, sticky = (E))


        #***************************** Left Frame **********************************************************
                                                
        self.LeftFrame = ttk.Frame(self.root, borderwidth=3, relief="sunken")
        self.LeftFrame.grid(column = 1, row = 1, sticky = (NS))


        #++++++++++++++++++++++++++++++  Box 1  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        self.counterFrame = ttk.Frame(self.LeftFrame,borderwidth=3, relief="sunken")
        self.counterFrame.grid(column = 0, row = 0, sticky = (N))


        # row 1  - labels
        L1_Label = ttk.Label(self.counterFrame, text = " L1 ")
        L1_Label.grid(column = 1, row = 0)
        L2_Label = ttk.Label(self.counterFrame, text = " L2 ")
        L2_Label.grid(column = 2, row = 0)
        Infusion_Label = ttk.Label(self.counterFrame, text = "Inf")
        Infusion_Label.grid(column = 3, row = 0)
        

        B1_ID_Label = ttk.Label(self.counterFrame, width=6, textvariable = self.B1_IDStr)
        B1_ID_Label.grid(column = 0, row = 1, pady=5)    
        B1_L1Resp_Label = ttk.Label(self.counterFrame, textvariable = str(self.B1_L1Resp))
        B1_L1Resp_Label.grid(column = 1, row = 1)
        B1_L2Resp_Label = ttk.Label(self.counterFrame, textvariable = str(self.B1_L2Resp))
        B1_L2Resp_Label.grid(column = 2, row = 1)
        B1_Inf_Label = ttk.Label(self.counterFrame, textvariable = str(self.B1_Inf))
        B1_Inf_Label.grid(column = 3, row = 1)      
        B1_Sched_Label = ttk.Label(self.counterFrame, width=10, textvariable = self.B1_sched)
        B1_Sched_Label.grid(column = 4, row = 1)
        B1_TimeLabel = ttk.Label(self.counterFrame, width=6, textvariable = self.B1_SessionTimeStr)
        B1_TimeLabel.grid(column = 5, row = 1)

        B2_ID_Label = ttk.Label(self.counterFrame, width=6, textvariable = self.B2_IDStr)
        B2_ID_Label.grid(column = 0, row = 2, pady=5)
        B2_L1Resp_Label = ttk.Label(self.counterFrame, textvariable = str(self.B2_L1Resp))
        B2_L1Resp_Label.grid(column = 1, row = 2)
        B2_L2Resp_Label = ttk.Label(self.counterFrame, textvariable = str(self.B2_L2Resp))
        B2_L2Resp_Label.grid(column = 2, row = 2)
        B2_Inf_Label = ttk.Label(self.counterFrame, textvariable = str(self.B2_Inf))
        B2_Inf_Label.grid(column = 3, row = 2)
        B2_Sched_Label = ttk.Label(self.counterFrame, width=10, textvariable = self.B2_sched)
        B2_Sched_Label.grid(column = 4, row = 2)        
        B2_TimeLabel = ttk.Label(self.counterFrame, width=6, textvariable = self.B2_SessionTimeStr)
        B2_TimeLabel.grid(column = 5, row = 2)
 
        B3_ID_Label = ttk.Label(self.counterFrame, width=6, textvariable = self.B3_IDStr)
        B3_ID_Label.grid(column = 0, row = 3, pady=5)
        B3_L1Resp_Label = ttk.Label(self.counterFrame, textvariable = str(self.B3_L1Resp))
        B3_L1Resp_Label.grid(column = 1, row = 3)
        B3_L2Resp_Label = ttk.Label(self.counterFrame, textvariable = str(self.B3_L2Resp))
        B3_L2Resp_Label.grid(column = 2, row = 3)
        B3_Inf_Label = ttk.Label(self.counterFrame, textvariable = str(self.B3_Inf))
        B3_Inf_Label.grid(column = 3, row = 3)
        B3_Sched_Label = ttk.Label(self.counterFrame, width=10, textvariable = self.B3_sched)
        B3_Sched_Label.grid(column = 4, row = 3)
        B3_TimeLabel = ttk.Label(self.counterFrame, width=6, textvariable = self.B3_SessionTimeStr)
        B3_TimeLabel.grid(column = 5, row = 3)

        B4_ID_Label = ttk.Label(self.counterFrame, width=6, textvariable = self.B4_IDStr)
        B4_ID_Label.grid(column = 0, row = 4, pady=5)
        B4_L1Resp_Label = ttk.Label(self.counterFrame, textvariable = str(self.B4_L1Resp))
        B4_L1Resp_Label.grid(column = 1, row = 4)
        B4_L2Resp_Label = ttk.Label(self.counterFrame, textvariable = str(self.B4_L2Resp))
        B4_L2Resp_Label.grid(column = 2, row = 4)
        B4_Inf_Label = ttk.Label(self.counterFrame, textvariable = str(self.B4_Inf))
        B4_Inf_Label.grid(column = 3, row = 4)
        B4_Sched_Label = ttk.Label(self.counterFrame, width=10, textvariable = self.B4_sched)
        B4_Sched_Label.grid(column = 4, row = 4)
        B4_TimeLabel = ttk.Label(self.counterFrame, width=6, textvariable = self.B4_SessionTimeStr)
        B4_TimeLabel.grid(column = 5, row = 4)

        B5_ID_Label = ttk.Label(self.counterFrame, width=6, textvariable = self.B5_IDStr)
        B5_ID_Label.grid(column = 0, row = 5, pady=5)
        B5_L1Resp_Label = ttk.Label(self.counterFrame, textvariable = str(self.B5_L1Resp))
        B5_L1Resp_Label.grid(column = 1, row = 5)        
        B5_L2Resp_Label = ttk.Label(self.counterFrame, textvariable = str(self.B5_L2Resp))
        B5_L2Resp_Label.grid(column = 2, row = 5)
        B5_Inf_Label = ttk.Label(self.counterFrame, textvariable = str(self.B5_Inf))
        B5_Inf_Label.grid(column = 3, row = 5)
        B5_Sched_Label = ttk.Label(self.counterFrame, width=10, textvariable = self.B5_sched)
        B5_Sched_Label.grid(column = 4, row = 5)
        B5_TimeLabel = ttk.Label(self.counterFrame, width=6, textvariable = self.B5_SessionTimeStr)
        B5_TimeLabel.grid(column = 5, row = 5)

        B6_ID_Label = ttk.Label(self.counterFrame, width=6, textvariable = self.B6_IDStr)
        B6_ID_Label.grid(column = 0, row = 6, pady=5)
        B6_L1Resp_Label = ttk.Label(self.counterFrame, textvariable = str(self.B6_L1Resp))
        B6_L1Resp_Label.grid(column = 1, row = 6)        
        B6_L2Resp_Label = ttk.Label(self.counterFrame, textvariable = str(self.B6_L2Resp))
        B6_L2Resp_Label.grid(column = 2, row = 6)
        B6_Inf_Label = ttk.Label(self.counterFrame, textvariable = str(self.B6_Inf))
        B6_Inf_Label.grid(column = 3, row = 6)
        B6_Sched_Label = ttk.Label(self.counterFrame, width=10, textvariable = self.B6_sched)
        B6_Sched_Label.grid(column = 4, row = 6)
        B6_TimeLabel = ttk.Label(self.counterFrame, width=6, textvariable = self.B6_SessionTimeStr)
        B6_TimeLabel.grid(column = 5, row = 6)

        B7_ID_Label = ttk.Label(self.counterFrame, width=6, textvariable = self.B7_IDStr)
        B7_ID_Label.grid(column = 0, row = 7, pady=5)
        B7_L1Resp_Label = ttk.Label(self.counterFrame, textvariable = str(self.B7_L1Resp))
        B7_L1Resp_Label.grid(column = 1, row = 7)
        B7_L2Resp_Label = ttk.Label(self.counterFrame, textvariable = str(self.B7_L2Resp))
        B7_L2Resp_Label.grid(column = 2, row = 7)
        B7_Inf_Label = ttk.Label(self.counterFrame, textvariable = str(self.B7_Inf))
        B7_Inf_Label.grid(column = 3, row = 7)
        B7_Sched_Label = ttk.Label(self.counterFrame, width=10, textvariable = self.B7_sched)
        B7_Sched_Label.grid(column = 4, row = 7)
        B7_TimeLabel = ttk.Label(self.counterFrame, width=6, textvariable = self.B7_SessionTimeStr)
        B7_TimeLabel.grid(column = 5, row = 7)

        B8_ID_Label = ttk.Label(self.counterFrame, width=6, textvariable = self.B8_IDStr)
        B8_ID_Label.grid(column = 0, row = 8, pady=5)
        B8_L1Resp_Label = ttk.Label(self.counterFrame, textvariable = str(self.B8_L1Resp))
        B8_L1Resp_Label.grid(column = 1, row = 8)
        B8_L2Resp_Label = ttk.Label(self.counterFrame, textvariable = str(self.B8_L2Resp))
        B8_L2Resp_Label.grid(column = 2, row = 8)
        B8_Inf_Label = ttk.Label(self.counterFrame, textvariable = str(self.B8_Inf))
        B8_Inf_Label.grid(column = 3, row = 8)
        B8_Sched_Label = ttk.Label(self.counterFrame, width=10, textvariable = self.B8_sched)
        B8_Sched_Label.grid(column = 4, row = 8)
        B8_TimeLabel = ttk.Label(self.counterFrame, width=6, textvariable = self.B8_SessionTimeStr)
        B8_TimeLabel.grid(column = 5, row = 8)


        # ********************** noteBookFrame *****************************************

        noteBookFrame = Frame(self.root, borderwidth=0, relief="sunken")
        noteBookFrame.grid(column = 0, row = 1)
        myNotebook = ttk.Notebook(noteBookFrame)
        self.iniTab = Frame(myNotebook)
        self.graphTab = Frame(myNotebook)
        self.diagnosticTab = Frame(myNotebook)
        myNotebook.add(self.iniTab,text = "Initialization")
        myNotebook.add(self.graphTab,text = "Graphs")      
        myNotebook.add(self.diagnosticTab,text = "Diagnostics")
        myNotebook.grid(row=0,column=0)

        # *********************  GraphTab **********************************************

        self.GraphFrame = ttk.Frame(self.graphTab,borderwidth=3, relief="sunken")
        self.GraphFrame.grid(column = 0, row = 0,rowspan=1,sticky = (N,E,W,S))
        
        self.graphButtonFrame = ttk.Frame(self.GraphFrame,borderwidth=3, relief="sunken")
        self.graphButtonFrame.grid(column = 0, row = 1, sticky = (N))
        B1_Radiobutton = ttk.Radiobutton(self.graphButtonFrame, text="Box 1", variable=self.selectedBox, value=0)
        B1_Radiobutton.grid(column = 0, row = 0)
        B2_Radiobutton = ttk.Radiobutton(self.graphButtonFrame, text="Box 2", variable=self.selectedBox, value=1)
        B2_Radiobutton.grid(column = 1, row = 0)
        B3_Radiobutton = ttk.Radiobutton(self.graphButtonFrame, text="Box 3", variable=self.selectedBox, value=2)
        B3_Radiobutton.grid(column = 2, row = 0)
        B4_Radiobutton = ttk.Radiobutton(self.graphButtonFrame, text="Box 4", variable=self.selectedBox, value=3)
        B4_Radiobutton.grid(column = 3, row = 0)
        B5_Radiobutton = ttk.Radiobutton(self.graphButtonFrame, text="Box 5", variable=self.selectedBox, value=4)
        B5_Radiobutton.grid(column = 4, row = 0)
        B6_Radiobutton = ttk.Radiobutton(self.graphButtonFrame, text="Box 6", variable=self.selectedBox, value=5)
        B6_Radiobutton.grid(column = 5, row = 0)
        B7_Radiobutton = ttk.Radiobutton(self.graphButtonFrame, text="Box 7", variable=self.selectedBox, value=6)
        B7_Radiobutton.grid(column = 6, row = 0)
        B8_Radiobutton = ttk.Radiobutton(self.graphButtonFrame, text="Box 8", variable=self.selectedBox, value=7)
        B8_Radiobutton.grid(column = 7, row = 0)
        example1_Radiobutton = ttk.Radiobutton(self.graphButtonFrame, text="PR", variable=self.selectedBox, value=8)
        example1_Radiobutton.grid(column = 8, row = 0)
        example2_Radiobutton = ttk.Radiobutton(self.graphButtonFrame, text="IntA", variable=self.selectedBox, value=9)
        example2_Radiobutton.grid(column = 9, row = 0)

        self.graphButtonFrame = ttk.Frame(self.GraphFrame,borderwidth=3, relief="sunken")
        self.graphButtonFrame.grid(column = 0, row = 2, columnspan = 10)
        topTimeStampButton = ttk.Button(self.graphButtonFrame,text="Timestamps",command=lambda: \
                                        self.drawAllTimeStamps(self.topCanvas, self.selectedBox.get(),self.selectMax_x_Scale.get()))
        topTimeStampButton.grid(column = 0, row = 0)
        topCumRecButton = ttk.Button(self.graphButtonFrame,text="Cum Rec",command=lambda: \
                                     self.drawCumulativeRecord(self.topCanvas, self.selectedBox.get(),self.selectMax_x_Scale.get()))
        topCumRecButton.grid(column = 1, row = 0)        
        topEventButton = ttk.Button(self.graphButtonFrame, text="Events",  command=lambda: self.drawEventRecords())
        topEventButton.grid(column = 2, row = 0)
        topClearButton = ttk.Button(self.graphButtonFrame,text="Clear Canvas", \
                                      command=lambda Canvas = 0: self.clearCanvas(Canvas))
        topClearButton.grid(column = 3, row = 0)

        self.topCanvas = Canvas(self.GraphFrame,width=self.CANVAS_WIDTH, height=self.CANVAS_HEIGHT)
        self.topCanvas.grid(column = 0, row = 3, columnspan = 10, stick = (W))
        self.topCanvas.create_text(150, 10, fill="blue", text="topCanvas")
    

        self.graphControlFrame = ttk.Frame(self.GraphFrame,borderwidth=3, relief="sunken")
        self.graphControlFrame.grid(column = 0, row = 4, columnspan = 10, sticky = (S))
        X_Axis_Label = ttk.Label(self.graphControlFrame, text="X Axis scales (min)")
        X_Axis_Label.grid(column = 0, row = 2)
        topRadiobutton30 = ttk.Radiobutton(self.graphControlFrame, text="10", variable=self.selectMax_x_Scale, value=10)
        topRadiobutton30.grid(column = 1, row = 2)
        topRadiobutton30 = ttk.Radiobutton(self.graphControlFrame, text="30", variable=self.selectMax_x_Scale, value=30)
        topRadiobutton30.grid(column = 2, row = 2)
        topRadiobutton60 = ttk.Radiobutton(self.graphControlFrame, text="60", variable=self.selectMax_x_Scale, value=60)
        topRadiobutton60.grid(column = 3, row = 2)
        topRadiobutton180 = ttk.Radiobutton(self.graphControlFrame, text="180", variable=self.selectMax_x_Scale, value=180)
        topRadiobutton180.grid(column = 4, row = 2)
        topRadiobutton180 = ttk.Radiobutton(self.graphControlFrame, text="360", variable=self.selectMax_x_Scale, value=360)
        topRadiobutton180.grid(column = 5, row = 2)




        """
        self.bottomCanvas = Canvas(self.GraphFrame,width=self.CANVAS_WIDTH, height=self.CANVAS_HEIGHT)
        self.bottomCanvas.grid(column = 1, row = 3, columnspan = 8)
       
        self.bottomCanvas.create_text(150, 10, fill="blue", text="bottomCanvas")
        bottomTimeStampButton = ttk.Button(self.GraphFrame,text="Timestamps",command=lambda: \
                                           self.drawAllTimeStamps(self.bottomCanvas, self.selectedBox.get(),self.selectMax_x_Scale.get()))
        bottomTimeStampButton.grid(column = 1, row = 4)
        bottomCumRecButton = ttk.Button(self.GraphFrame,text="Cum Rec",command=lambda: \
                                     self.drawCumulativeRecord(self.bottomCanvas, self.selectedBox.get(),self.selectMax_x_Scale.get()))
        bottomCumRecButton.grid(column = 2, row = 4)
        bottomEventButton = ttk.Button(self.GraphFrame, text="Events", command=lambda canvasNum = 1: self.drawEventRecords(canvasNum))
        bottomEventButton.grid(column = 3, row = 4)
        bottomClearButton = ttk.Button(self.GraphFrame,text="Clear Canvas", \
                                      command=lambda Canvas = 1: self.clearCanvas(Canvas))
        bottomClearButton.grid(column = 4, row = 4)
        """

        # ********************** Text Boxes - DiagnosticsTab ****************************************
        self.diagnosticFrame = ttk.Frame(self.diagnosticTab,borderwidth=3, relief="sunken")
        self.diagnosticFrame.grid(column = 0, row = 0,rowspan=1, sticky = (N,E,W,S))

        self.diagnostic_IO_Frame = ttk.Frame(self.diagnosticFrame,borderwidth=3, relief="sunken")
        self.diagnostic_IO_Frame.grid(column = 0, row = 0, columnspan = 2, sticky = (W))
        B1_Lever1CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "L1", variable = self.B1_lever1CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.moveLever1(0,self.B1_lever1CheckVar.get()))
        B1_Lever1CheckButton.grid(column = 1, row = 0)
        B1_Lever2CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "L2", variable = self.B1_lever2CheckVar, \
             onvalue = True, offvalue = False, command=lambda: self.moveLever2(0,self.B1_lever2CheckVar.get()))
        B1_Lever2CheckButton.grid(column = 2, row = 0)
        B1_PumpCheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "Pump", variable = self.B1_pumpCheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.togglePump(0,self.B1_pumpCheckVar.get()))
        B1_PumpCheckButton.grid(column = 3, row = 0)
        B1_LED1_CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "LED1", variable = self.B1_LED1CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.toggleLED1(0,self.B1_LED1CheckVar.get()))
        B1_LED1_CheckButton.grid(column = 4, row = 0)
        B1_LED2_CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "LED2", variable = self.B1_LED2CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.toggleLED2(0,self.B1_LED2CheckVar.get()))
        B1_LED2_CheckButton.grid(column = 5, row = 0)       
        B2_Lever1CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "L1", variable = self.B2_lever1CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.moveLever1(1,self.B2_lever1CheckVar.get()))
        B2_Lever1CheckButton.grid(column = 1, row = 1)
        B2_Lever2CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "L2", variable = self.B2_lever2CheckVar, \
             onvalue = True, offvalue = False, command=lambda: self.moveLever2(1,self.B2_lever2CheckVar.get()))
        B2_Lever2CheckButton.grid(column = 2, row = 1)
        B2_PumpCheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "Pump", variable = self.B2_pumpCheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.togglePump(1,self.B2_pumpCheckVar.get()))
        B2_PumpCheckButton.grid(column = 3, row = 1)
        B2_LED1_CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "LED1", variable = self.B2_LED1CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.toggleLED1(1,self.B2_LED1CheckVar.get()))
        B2_LED1_CheckButton.grid(column = 4, row = 1)
        B2_LED2_CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "LED2", variable = self.B2_LED2CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.toggleLED2(1,self.B2_LED2CheckVar.get()))
        B2_LED2_CheckButton.grid(column = 5, row = 1)        
        B3_Lever1CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "L1", variable = self.B3_lever1CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.moveLever1(2,self.B3_lever1CheckVar.get()))
        B3_Lever1CheckButton.grid(column = 1, row = 2)
        B3_Lever2CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "L2", variable = self.B3_lever2CheckVar, \
             onvalue = True, offvalue = False, command=lambda: self.moveLever2(2,self.B3_lever2CheckVar.get()))
        B3_Lever2CheckButton.grid(column = 2, row = 2)
        B3_PumpCheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "Pump", variable = self.B3_pumpCheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.togglePump(2,self.B3_pumpCheckVar.get()))
        B3_PumpCheckButton.grid(column = 3, row = 2)
        B3_LED1_CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "LED1", variable = self.B3_LED1CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.toggleLED1(2,self.B3_LED1CheckVar.get()))
        B3_LED1_CheckButton.grid(column = 4, row = 2)
        B3_LED2_CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "LED2", variable = self.B3_LED2CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.toggleLED2(2,self.B3_LED2CheckVar.get()))
        B3_LED2_CheckButton.grid(column = 5, row = 2)        
        B4_Lever1CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "L1", variable = self.B4_lever1CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.moveLever1(3,self.B4_lever1CheckVar.get()))
        B4_Lever1CheckButton.grid(column = 1, row = 3)
        B4_Lever2CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "L2", variable = self.B4_lever2CheckVar, \
             onvalue = True, offvalue = False, command=lambda: self.moveLever2(3,self.B4_lever2CheckVar.get()))
        B4_Lever2CheckButton.grid(column = 2, row = 3)
        B4_PumpCheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "Pump", variable = self.B4_pumpCheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.togglePump(3,self.B4_pumpCheckVar.get()))
        B4_PumpCheckButton.grid(column = 3, row = 3)
        B4_LED1_CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "LED1", variable = self.B4_LED1CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.toggleLED1(3,self.B4_LED1CheckVar.get()))
        B4_LED1_CheckButton.grid(column = 4, row = 3)
        B4_LED2_CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "LED2", variable = self.B4_LED2CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.toggleLED2(3,self.B4_LED2CheckVar.get()))
        B4_LED2_CheckButton.grid(column = 5, row = 3)
        B5_Lever1CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "L1", variable = self.B5_lever1CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.moveLever1(4,self.B5_lever1CheckVar.get()))
        B5_Lever1CheckButton.grid(column = 1, row = 4)
        B5_Lever2CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "L2", variable = self.B5_lever2CheckVar, \
             onvalue = True, offvalue = False, command=lambda: self.moveLever2(4,self.B5_lever2CheckVar.get()))
        B5_Lever2CheckButton.grid(column = 2, row = 4)
        B5_PumpCheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "Pump", variable = self.B5_pumpCheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.togglePump(4,self.B5_pumpCheckVar.get()))
        B5_PumpCheckButton.grid(column = 3, row = 4)
        B5_LED1_CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "LED1", variable = self.B5_LED1CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.toggleLED1(4,self.B5_LED1CheckVar.get()))
        B5_LED1_CheckButton.grid(column = 4, row = 4)
        B5_LED2_CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "LED2", variable = self.B5_LED2CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.toggleLED2(4,self.B5_LED2CheckVar.get()))
        B5_LED2_CheckButton.grid(column = 5, row = 4)
        B6_Lever1CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "L1", variable = self.B6_lever1CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.moveLever1(5,self.B6_lever1CheckVar.get()))
        B6_Lever1CheckButton.grid(column = 1, row = 5)
        B6_Lever2CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "L2", variable = self.B6_lever2CheckVar, \
             onvalue = True, offvalue = False, command=lambda: self.moveLever2(5,self.B6_lever2CheckVar.get()))
        B6_Lever2CheckButton.grid(column = 2, row = 5)
        B6_PumpCheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "Pump", variable = self.B6_pumpCheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.togglePump(5,self.B6_pumpCheckVar.get()))
        B6_PumpCheckButton.grid(column = 3, row = 5)
        B6_LED1_CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "LED1", variable = self.B6_LED1CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.toggleLED1(5,self.B6_LED1CheckVar.get()))
        B6_LED1_CheckButton.grid(column = 4, row = 5)
        B6_LED2_CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "LED2", variable = self.B6_LED2CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.toggleLED2(5,self.B6_LED2CheckVar.get()))
        B6_LED2_CheckButton.grid(column = 5, row = 5) 
        B7_Lever1CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "L1", variable = self.B7_lever1CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.moveLever1(6,self.B7_lever1CheckVar.get()))
        B7_Lever1CheckButton.grid(column = 1, row = 6)
        B7_Lever2CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "L2", variable = self.B7_lever2CheckVar, \
             onvalue = True, offvalue = False, command=lambda: self.moveLever2(6,self.B7_lever2CheckVar.get()))
        B7_Lever2CheckButton.grid(column = 2, row = 6)
        B7_PumpCheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "Pump", variable = self.B7_pumpCheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.togglePump(6,self.B7_pumpCheckVar.get()))
        B7_PumpCheckButton.grid(column = 3, row = 6)        
        B7_LED1_CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "LED1", variable = self.B7_LED1CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.toggleLED1(6,self.B7_LED1CheckVar.get()))
        B7_LED1_CheckButton.grid(column = 4, row = 6)
        B7_LED2_CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "LED2", variable = self.B7_LED2CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.toggleLED2(6,self.B7_LED2CheckVar.get()))
        B7_LED2_CheckButton.grid(column = 5, row = 6)         
        B8_Lever1CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "L1", variable = self.B8_lever1CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.moveLever1(7,self.B8_lever1CheckVar.get()))
        B8_Lever1CheckButton.grid(column = 1, row = 7)
        B8_Lever2CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "L2", variable = self.B8_lever2CheckVar, \
             onvalue = True, offvalue = False, command=lambda: self.moveLever2(7,self.B8_lever2CheckVar.get()))
        B8_Lever2CheckButton.grid(column = 2, row = 7)
        B8_PumpCheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "Pump", variable = self.B8_pumpCheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.togglePump(7,self.B8_pumpCheckVar.get()))
        B8_PumpCheckButton.grid(column = 3, row = 7)
        B8_LED1_CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "LED1", variable = self.B8_LED1CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.toggleLED1(7,self.B8_LED1CheckVar.get()))
        B8_LED1_CheckButton.grid(column = 4, row = 7)
        B8_LED2_CheckButton = Checkbutton(self.diagnostic_IO_Frame, text = "LED2", variable = self.B8_LED2CheckVar, \
                onvalue = True, offvalue = False, command=lambda: self.toggleLED2(7,self.B8_LED2CheckVar.get()))
        B8_LED2_CheckButton.grid(column = 5, row = 7)

        self.startStopButtonFrame = ttk.Frame(self.diagnosticFrame,borderwidth=3, relief="sunken")
        self.startStopButtonFrame.grid(column = 2, row = 0, sticky = (N))
        B1_StartButton = ttk.Button(self.startStopButtonFrame,text="Start", \
                                   command=lambda boxIndex = 0: self.startSession(boxIndex))
        B1_StartButton.grid(column = 0, row = 0)
        B1_StopButton = ttk.Button(self.startStopButtonFrame,text="Stop", \
                                   command=lambda boxIndex = 0: self.stopSession(boxIndex))
        B1_StopButton.grid(column = 1, row = 0) 
        B2_StartButton = ttk.Button(self.startStopButtonFrame,text="Start", \
                                   command=lambda boxIndex = 1: self.startSession(boxIndex))
        B2_StartButton.grid(column = 0, row = 1)
        B2_StopButton = ttk.Button(self.startStopButtonFrame,text="Stop", \
                                   command=lambda boxIndex = 1: self.stopSession(boxIndex))
        B2_StopButton.grid(column = 1, row = 1)
        B3_StartButton = ttk.Button(self.startStopButtonFrame,text="Start", \
                                   command=lambda boxIndex = 2: self.startSession(boxIndex))
        B3_StartButton.grid(column = 0, row = 2)
        B3_StopButton = ttk.Button(self.startStopButtonFrame,text="Stop", \
                                   command=lambda boxIndex = 2: self.stopSession(boxIndex))
        B3_StopButton.grid(column = 1, row = 2)
        B4_StartButton = ttk.Button(self.startStopButtonFrame,text="Start", \
                                   command=lambda boxIndex = 3: self.startSession(boxIndex))
        B4_StartButton.grid(column = 0, row = 3)
        B4_StopButton = ttk.Button(self.startStopButtonFrame,text="Stop", \
                                   command=lambda boxIndex = 3: self.stopSession(boxIndex))
        B4_StopButton.grid(column = 1, row = 3)
        B5_StartButton = ttk.Button(self.startStopButtonFrame,text="Start", \
                                   command=lambda boxIndex = 4: self.startSession(boxIndex))
        B5_StartButton.grid(column = 0, row = 4)
        B5_StopButton = ttk.Button(self.startStopButtonFrame,text="Stop", \
                                   command=lambda boxIndex = 4: self.stopSession(boxIndex))
        B5_StopButton.grid(column = 1, row = 4)
        B6_StartButton = ttk.Button(self.startStopButtonFrame,text="Start", \
                                   command=lambda boxIndex = 5: self.startSession(boxIndex))
        B6_StartButton.grid(column = 0, row = 5)
        B6_StopButton = ttk.Button(self.startStopButtonFrame,text="Stop", \
                                   command=lambda boxIndex = 5: self.stopSession(boxIndex))
        B6_StopButton.grid(column = 1, row = 5)
        B7_StartButton = ttk.Button(self.startStopButtonFrame,text="Start", \
                                   command=lambda boxIndex = 6: self.startSession(boxIndex))
        B7_StartButton.grid(column = 0, row = 6)
        B7_StopButton = ttk.Button(self.startStopButtonFrame,text="Stop", \
                                   command=lambda boxIndex = 6: self.stopSession(boxIndex))
        B7_StopButton.grid(column = 1, row = 6)
        B8_StartButton = ttk.Button(self.startStopButtonFrame,text="Start", \
                                   command=lambda boxIndex = 7: self.startSession(boxIndex))
        B8_StartButton.grid(column = 0, row = 7)
        B8_StopButton = ttk.Button(self.startStopButtonFrame,text="Stop", \
                                   command=lambda boxIndex = 7: self.stopSession(boxIndex))
        B8_StopButton.grid(column = 1, row = 7) 


        self.diagnosticButtonFrame = ttk.Frame(self.diagnosticFrame,borderwidth=3, relief="sunken")
        self.diagnosticButtonFrame.grid(column = 0, row = 1)
        

        checkLeversCheckButton = Checkbutton(self.diagnosticButtonFrame, text = "Check Levers", variable = self.checkLeversCheckVar, \
                    onvalue = True, offvalue = False, command=lambda: self.toggleCheckLevers(self.checkLeversCheckVar.get()))       
        checkLeversCheckButton.grid(column = 0, row = 0, sticky = (EW))
        B1_L1_TestButton = ttk.Button(self.diagnosticButtonFrame,text="L1 test", command=lambda: self.mimicL1Response(0))
        B1_L1_TestButton.grid(column = 0, row = 1)
        B2_L1_TestButton = ttk.Button(self.diagnosticButtonFrame,text="L2 test", command=lambda: self.mimicL1Response(1))
        B2_L1_TestButton.grid(column = 0, row = 2)
        B3_L1_TestButton = ttk.Button(self.diagnosticButtonFrame,text="L3 test", command=lambda: self.mimicL1Response(2))
        B3_L1_TestButton.grid(column = 0, row = 3)
        B4_L1_TestButton = ttk.Button(self.diagnosticButtonFrame,text="L4 test", command=lambda: self.mimicL1Response(3))
        B4_L1_TestButton.grid(column = 0, row = 4)
        B5_L1_TestButton = ttk.Button(self.diagnosticButtonFrame,text="L5 test", command=lambda: self.mimicL1Response(4))
        B5_L1_TestButton.grid(column = 0, row = 5)
        B6_L1_TestButton = ttk.Button(self.diagnosticButtonFrame,text="L6 test", command=lambda: self.mimicL1Response(5))
        B6_L1_TestButton.grid(column = 0, row = 6)
        B7_L1_TestButton = ttk.Button(self.diagnosticButtonFrame,text="L7 test", command=lambda: self.mimicL1Response(6))
        B7_L1_TestButton.grid(column = 0, row = 7)
        B8_L1_TestButton = ttk.Button(self.diagnosticButtonFrame,text="L8 test", command=lambda: self.mimicL1Response(7))
        B8_L1_TestButton.grid(column = 0, row = 8)

        testButton = ttk.Button(self.diagnosticButtonFrame,text="MaxDelta",command = self.testFunction)
        testButton.grid(column = 0, row = 9)
        testButton4 = ttk.Button(self.diagnosticButtonFrame,text="Block Time 0",command = self.testFunction4)
        testButton4.grid(column = 0, row = 10)
        testButton5 = ttk.Button(self.diagnosticButtonFrame,text="Block Time 1",command = self.testFunction5)
        testButton5.grid(column = 0, row = 11)
        testButton6 = ttk.Button(self.diagnosticButtonFrame,text="Diagnostics",command = self.testFunction6)
        testButton6.grid(column = 0, row = 12)
        
        self.topTextFrame = ttk.Frame(self.diagnosticFrame,borderwidth=3, relief="sunken")
        self.topTextFrame.grid(column = 1, row = 1)
        showDataStreamCheckButton = Checkbutton(self.topTextFrame, text = "Show Data Steam", variable = self.showDataStreamCheckVar, \
                onvalue = True, offvalue = False)
        showDataStreamCheckButton.grid(column = 0, row = 0, sticky = (EW))       
        self.topTextbox = tkinter.scrolledtext.ScrolledText(self.topTextFrame,height=20,width= 25)
        self.topTextbox.grid(column = 0, row = 1,sticky = (N))
        self.topTextbox.insert('1.0',"\n")
        self.bottomTextFrame = ttk.Frame(self.diagnosticFrame,borderwidth=3, relief="sunken")
        self.bottomTextFrame.grid(column = 2, row = 1)
        showOutputStreamCheckButton = Checkbutton(self.bottomTextFrame, text = "Show Output", variable = self.showOutputCheckVar, \
                onvalue = True, offvalue = False)
        showOutputStreamCheckButton.grid(column = 0, row = 0, sticky = (EW)) 
        self.bottomTextbox = tkinter.scrolledtext.ScrolledText(self.bottomTextFrame,height=20,width= 25)
        self.bottomTextbox.grid(column = 0, row = 1,sticky = (N))        
        self.bottomTextbox.insert('1.0',"Bottom Text Box\n")

        pumpChaosCheckButton = Checkbutton(self.diagnosticFrame, text = "Pump Chaos - Extremely Dangerous", variable = self.pumpChaosCheckVar, \
                onvalue = True, offvalue = False)       
        pumpChaosCheckButton.grid(column = 0, row = 2)

        # ********************** Lists used to read and write to initialization file ******************
        self.IDStrList = [self.B1_IDStr, self.B2_IDStr, self.B3_IDStr, self.B4_IDStr, \
                          self.B5_IDStr, self.B6_IDStr, self.B7_IDStr, self.B8_IDStr]
        self.WeightList = [self.B1_Weight,self.B2_Weight,self.B3_Weight,self.B4_Weight, \
                           self.B5_Weight,self.B6_Weight,self.B7_Weight,self.B8_Weight]
        self.schedList = [self.B1_sched,self.B2_sched,self.B3_sched,self.B4_sched, \
                          self.B5_sched,self.B6_sched,self.B7_sched,self.B8_sched, ]
        self.FR_valueList = [self.B1_FR_value,self.B2_FR_value,self.B3_FR_value,self.B4_FR_value, \
                             self.B5_FR_value,self.B6_FR_value,self.B7_FR_value,self.B8_FR_value,]
        self.SessionLengthList = [self.B1_SessionLength,self.B2_SessionLength,self.B3_SessionLength,self.B4_SessionLength, \
                                  self.B5_SessionLength,self.B6_SessionLength,self.B7_SessionLength,self.B8_SessionLength]
        self.PumpTimeList = [self.B1_PumpTime,self.B2_PumpTime,self.B3_PumpTime,self.B4_PumpTime, \
                             self.B5_PumpTime,self.B6_PumpTime,self.B7_PumpTime,self.B8_PumpTime,]
        self.calcPumpTimeList = [self.B1_calcPumpTime,self.B2_calcPumpTime,self.B3_calcPumpTime,self.B4_calcPumpTime, \
                                 self.B5_calcPumpTime,self.B6_calcPumpTime,self.B7_calcPumpTime,self.B8_calcPumpTime,]

        # ********************* Lists used in periodic check  ****************************************

        self.B1_boolVarList = [self.B1_lever1CheckVar, self.B1_lever2CheckVar, self.B1_pumpCheckVar, self.B1_LED1CheckVar, self.B1_LED2CheckVar]
        self.B2_boolVarList = [self.B2_lever1CheckVar, self.B2_lever2CheckVar, self.B2_pumpCheckVar, self.B2_LED1CheckVar, self.B2_LED2CheckVar]
        self.B3_boolVarList = [self.B3_lever1CheckVar, self.B3_lever2CheckVar, self.B3_pumpCheckVar, self.B3_LED1CheckVar, self.B3_LED2CheckVar]
        self.B4_boolVarList = [self.B4_lever1CheckVar, self.B4_lever2CheckVar, self.B4_pumpCheckVar, self.B4_LED1CheckVar, self.B4_LED2CheckVar]
        self.B5_boolVarList = [self.B5_lever1CheckVar, self.B5_lever2CheckVar, self.B5_pumpCheckVar, self.B5_LED1CheckVar, self.B5_LED2CheckVar]
        self.B6_boolVarList = [self.B6_lever1CheckVar, self.B6_lever2CheckVar, self.B6_pumpCheckVar, self.B6_LED1CheckVar, self.B6_LED2CheckVar]
        self.B7_boolVarList = [self.B7_lever1CheckVar, self.B7_lever2CheckVar, self.B7_pumpCheckVar, self.B7_LED1CheckVar, self.B7_LED2CheckVar]
        self.B8_boolVarList = [self.B8_lever1CheckVar, self.B8_lever2CheckVar, self.B8_pumpCheckVar, self.B8_LED1CheckVar, self.B8_LED2CheckVar]
        
        self.boxesVarLists = [self.B1_boolVarList,self.B2_boolVarList,self.B3_boolVarList,self.B4_boolVarList, \
                              self.B5_boolVarList,self.B6_boolVarList,self.B7_boolVarList,self.B8_boolVarList]
        self.L1ResponsesList = [self.B1_L1Resp,self.B2_L1Resp,self.B3_L1Resp,self.B4_L1Resp, \
                              self.B5_L1Resp,self.B6_L1Resp,self.B7_L1Resp,self.B8_L1Resp]
        self.L2ResponsesList = [self.B1_L2Resp,self.B2_L2Resp,self.B3_L2Resp,self.B4_L2Resp, \
                              self.B5_L2Resp,self.B6_L2Resp,self.B7_L2Resp,self.B8_L2Resp]
        self.InfList = [self.B1_Inf,self.B2_Inf,self.B3_Inf,self.B4_Inf, \
                        self.B5_Inf,self.B6_Inf,self.B7_Inf,self.B8_Inf]

        self.sessionTimeStrList = [self.B1_SessionTimeStr, self.B2_SessionTimeStr,self.B3_SessionTimeStr, self.B4_SessionTimeStr, \
                                   self.B5_SessionTimeStr, self.B6_SessionTimeStr,self.B7_SessionTimeStr, self.B8_SessionTimeStr]
        
        # ********************* Initialization Parameters ****************************

        INI_Frame = ttk.Frame(self.iniTab, borderwidth = 3, relief="sunken")
        INI_Frame.grid(column = 0, row = 0)
        
        """
        The GUIClass has lists of ttk variables (IntVar or textvariable) which 
        map onto variables in each Box object.
        ID, weight, sched (1..4), pumptime, calcPumpTime (boolean), sessionlength

        """
        B1_ROW_ =  ttk.Label(INI_Frame, text="BOX 1").grid(column=0, row=0, sticky=(W, E))
        B1_ID_Entry = ttk.Entry(INI_Frame, width=6,textvariable=self.B1_IDStr)
        B1_ID_Entry.grid(column = 1, row = 0) 
        B1_Weight_Entry = ttk.Entry(INI_Frame, width=6,textvariable=self.B1_Weight)                                                 
        B1_Weight_Entry.grid(column = 2, row = 0)
        B1_Ratio = Spinbox(INI_Frame, textvariable = self.B1_FR_value, width = 3, from_=1, to=1000)
        B1_Ratio.grid(column=4, row=0)
        B1_SchedComboBox = ttk.Combobox(INI_Frame, textvariable=self.B1_sched, width = 12)
        B1_SchedComboBox.grid(column=5, row=0)
        B1_SchedComboBox['values'] = self.sched 
        B1_SessionLength = Spinbox(INI_Frame, textvariable = self.B1_SessionLength, width = 3, values = [2, 5, 30, 60, 90, 120, 180, 240, 300, 360])
        B1_SessionLength.grid(column=8, row=0)
        B1_SessionLabel_ =  ttk.Label(INI_Frame, text="Min").grid(column=9, row=0, sticky=(W, E))
        B1_PumpTime = Spinbox(INI_Frame, textvariable = str(self.B1_PumpTime), width = 3, from_=300, to=600)
        B1_PumpTime.grid(column=10,row=0)
        B1_CalcPumpTime = Checkbutton(INI_Frame, text = "Calc  ", variable = self.B1_calcPumpTime, \
                onvalue = True, offvalue = False, command=lambda boxIndex = 0: self.calcPumpTime(self.B1_calcPumpTime.get(),boxIndex))
        B1_CalcPumpTime.grid(column=11, row=0)


        B2_ROW_ =  ttk.Label(INI_Frame, text="BOX 2").grid(column=0, row=1, sticky=(W, E))
        B2_ID_Entry = ttk.Entry(INI_Frame, width=6,textvariable=self.B2_IDStr)
        B2_ID_Entry.grid(column = 1, row=1) 
        B2_Weight_Entry = ttk.Entry(INI_Frame, width=6,textvariable=self.B2_Weight)                                                 
        B2_Weight_Entry.grid(column = 2, row=1)
        B2_Ratio = Spinbox(INI_Frame, textvariable = self.B2_FR_value, width = 3, from_=1, to=1000)
        B2_Ratio.grid(column=4, row=1)
        B2_SchedComboBox = ttk.Combobox(INI_Frame, textvariable=self.B2_sched, width = 12)
        B2_SchedComboBox.grid(column=5, row=1)
        B2_SchedComboBox['values'] = self.sched
        B2_SessionLength = Spinbox(INI_Frame, textvariable = self.B2_SessionLength, width = 3, values = [2, 5, 30, 60, 90, 120, 180, 240, 300, 360])
        B2_SessionLength.grid(column=8, row=1)
        B2_SessionLabel_ =  ttk.Label(INI_Frame, text="Min").grid(column=9, row=1, sticky=(W, E))
        B2_PumpTime = Spinbox(INI_Frame, textvariable = str(self.B2_PumpTime), width = 3, from_=300, to=600)
        B2_PumpTime.grid(column=10,row=1)
        B2_CalcPumpTime = Checkbutton(INI_Frame, text = "Calc  ", variable = self.B2_calcPumpTime, \
                onvalue = True, offvalue = False, command=lambda boxIndex = 1: self.calcPumpTime(self.B2_calcPumpTime.get(),boxIndex))
        B2_CalcPumpTime.grid(column=11, row=1)


        B3_ROW_ =  ttk.Label(INI_Frame, text="BOX 3").grid(column=0, row=2, sticky=(W, E))
        B3_ID_Entry = ttk.Entry(INI_Frame, width=6,textvariable=self.B3_IDStr)
        B3_ID_Entry.grid(column = 1, row=2) 
        B3_Weight_Entry = ttk.Entry(INI_Frame, width=6,textvariable=self.B3_Weight)                                                 
        B3_Weight_Entry.grid(column = 2, row=2)
        B3_Ratio = Spinbox(INI_Frame, textvariable = self.B3_FR_value, width = 3, from_=1, to=1000)
        B3_Ratio.grid(column=4, row=2)
        B3_SchedComboBox = ttk.Combobox(INI_Frame, textvariable=self.B3_sched, width = 12)
        B3_SchedComboBox.grid(column=5, row=2)
        B3_SchedComboBox['values'] = self.sched
        B3_SessionLength = Spinbox(INI_Frame, textvariable = self.B3_SessionLength, width = 3, values = [30, 60, 90, 120, 180, 240, 300, 360])
        B3_SessionLength.grid(column=8, row=2)
        B3_SessionLabel_ =  ttk.Label(INI_Frame, text="Min").grid(column=9, row=2, sticky=(W, E))
        B3_PumpTime = Spinbox(INI_Frame, textvariable = str(self.B3_PumpTime), width = 3, from_=300, to=600)
        B3_PumpTime.grid(column=10,row=2)
        B3_CalcPumpTime = Checkbutton(INI_Frame, text = "Calc  ", variable = self.B3_calcPumpTime, \
                onvalue = True, offvalue = False, command=lambda boxIndex = 2: self.calcPumpTime(self.B3_calcPumpTime.get(),boxIndex))
        B3_CalcPumpTime.grid(column=11, row=2)

        B4_ROW_ =  ttk.Label(INI_Frame, text="BOX 4").grid(column=0, row=3, sticky=(W, E))
        B4_ID_Entry = ttk.Entry(INI_Frame, width=6,textvariable=self.B4_IDStr)
        B4_ID_Entry.grid(column = 1, row=3) 
        B4_Weight_Entry = ttk.Entry(INI_Frame, width=6,textvariable=self.B4_Weight)                                                 
        B4_Weight_Entry.grid(column = 2, row=3)
        B4_Ratio = Spinbox(INI_Frame, textvariable = self.B4_FR_value, width = 3, from_=1, to=1000)
        B4_Ratio.grid(column=4, row=3)
        B4_SchedComboBox = ttk.Combobox(INI_Frame, textvariable=self.B4_sched, width = 12)
        B4_SchedComboBox.grid(column=5, row=3)
        B4_SchedComboBox['values'] = self.sched
        B4_SessionLength = Spinbox(INI_Frame, textvariable = self.B4_SessionLength, width = 3, values = [30, 60, 90, 120, 180, 240, 300, 360])
        B4_SessionLength.grid(column=8, row=3)
        B4_SessionLabel_ =  ttk.Label(INI_Frame, text="Min").grid(column=9, row=3, sticky=(W, E))
        B4_PumpTime = Spinbox(INI_Frame, textvariable = str(self.B4_PumpTime), width = 3, from_=300, to=600)
        B4_PumpTime.grid(column=10,row=3)
        B4_CalcPumpTime = Checkbutton(INI_Frame, text = "Calc  ", variable = self.B4_calcPumpTime, \
                onvalue = True, offvalue = False, command=lambda boxIndex = 3: self.calcPumpTime(self.B4_calcPumpTime.get(),boxIndex))
        B4_CalcPumpTime.grid(column=11, row=3)

        B5_ROW_ =  ttk.Label(INI_Frame, text="BOX 5").grid(column=0, row=4, sticky=(W, E))
        B5_ID_Entry = ttk.Entry(INI_Frame, width=6,textvariable=self.B5_IDStr)
        B5_ID_Entry.grid(column = 1, row = 4) 
        B5_Weight_Entry = ttk.Entry(INI_Frame, width=6,textvariable=self.B5_Weight)                                                 
        B5_Weight_Entry.grid(column = 2, row = 4)
        B5_Ratio = Spinbox(INI_Frame, textvariable = self.B5_FR_value, width = 3, from_=1, to=1000)
        B5_Ratio.grid(column=4, row=4)
        B5_SchedComboBox = ttk.Combobox(INI_Frame, textvariable=self.B5_sched, width = 12)
        B5_SchedComboBox.grid(column=5, row=4)
        B5_SchedComboBox['values'] = self.sched
        B5_SessionLength = Spinbox(INI_Frame, textvariable = self.B5_SessionLength, width = 3, values = [30, 60, 90, 120, 180, 240, 300, 360])
        B5_SessionLength.grid(column=8, row=4)
        B5_SessionLabel_ =  ttk.Label(INI_Frame, text="Min").grid(column=9, row=4, sticky=(W, E))
        B5_PumpTime = Spinbox(INI_Frame, textvariable = str(self.B5_PumpTime), width = 3, from_=300, to=600)
        B5_PumpTime.grid(column=10,row=4)
        B5_CalcPumpTime = Checkbutton(INI_Frame, text = "Calc  ", variable = self.B5_calcPumpTime, \
                onvalue = True, offvalue = False, command=lambda boxIndex = 4: self.calcPumpTime(self.B5_calcPumpTime.get(),boxIndex))
        B5_CalcPumpTime.grid(column=11, row=4)
        
        B6_ROW_ =  ttk.Label(INI_Frame, text="BOX 6").grid(column=0, row=5, sticky=(W, E))
        B6_ID_Entry = ttk.Entry(INI_Frame, width=6,textvariable=self.B6_IDStr)
        B6_ID_Entry.grid(column = 1, row=5) 
        B6_Weight_Entry = ttk.Entry(INI_Frame, width=6,textvariable=self.B6_Weight)                                                 
        B6_Weight_Entry.grid(column = 2, row=5)
        B6_Ratio = Spinbox(INI_Frame, textvariable = self.B6_FR_value, width = 3, from_=1, to=1000)
        B6_Ratio.grid(column=4, row=5)
        B6_SchedComboBox = ttk.Combobox(INI_Frame, textvariable=self.B6_sched, width = 12)
        B6_SchedComboBox.grid(column=5, row=5)
        B6_SchedComboBox['values'] = self.sched
        B6_SessionLength = Spinbox(INI_Frame, textvariable = self.B6_SessionLength, width = 3, values = [30, 60, 90, 120, 180, 240, 300, 360])
        B6_SessionLength.grid(column=8, row=5)
        B6_SessionLabel_ =  ttk.Label(INI_Frame, text="Min").grid(column=9, row=5, sticky=(W, E))
        B6_PumpTime = Spinbox(INI_Frame, textvariable = str(self.B6_PumpTime), width = 3, from_=300, to=600)
        B6_PumpTime.grid(column=10,row=5)
        B6_CalcPumpTime = Checkbutton(INI_Frame, text = "Calc  ", variable = self.B6_calcPumpTime, \
                onvalue = True, offvalue = False, command=lambda boxIndex = 5: self.calcPumpTime(self.B6_calcPumpTime.get(),boxIndex))
        B6_CalcPumpTime.grid(column=11, row=5)

        B7_ROW_ =  ttk.Label(INI_Frame, text="BOX 7").grid(column=0, row=6, sticky=(W, E))
        B7_ID_Entry = ttk.Entry(INI_Frame, width=6,textvariable=self.B7_IDStr)
        B7_ID_Entry.grid(column = 1, row=6) 
        B7_Weight_Entry = ttk.Entry(INI_Frame, width=6,textvariable=self.B7_Weight)                                                 
        B7_Weight_Entry.grid(column = 2, row=6)
        B7_Ratio = Spinbox(INI_Frame, textvariable = self.B7_FR_value, width = 3, from_=1, to=1000)
        B7_Ratio.grid(column=4, row=6)
        B7_SchedComboBox = ttk.Combobox(INI_Frame, textvariable=self.B7_sched, width = 12)
        B7_SchedComboBox.grid(column=5, row=6)
        B7_SchedComboBox['values'] = self.sched
        B7_SessionLength = Spinbox(INI_Frame, textvariable = self.B7_SessionLength, width = 3, values = [30, 60, 90, 120, 180, 240, 300, 360])
        B7_SessionLength.grid(column=8, row=6)
        B7_SessionLabel_ =  ttk.Label(INI_Frame, text="Min").grid(column=9, row=6, sticky=(W, E))
        B7_PumpTime = Spinbox(INI_Frame, textvariable = str(self.B7_PumpTime), width = 3, from_=300, to=600)
        B7_PumpTime.grid(column=10,row=6)
        B7_CalcPumpTime = Checkbutton(INI_Frame, text = "Calc  ", variable = self.B7_calcPumpTime, \
                onvalue = True, offvalue = False, command=lambda boxIndex = 6: self.calcPumpTime(self.B7_calcPumpTime.get(),boxIndex))
        B7_CalcPumpTime.grid(column=11, row=6)

        B8_ROW_ =  ttk.Label(INI_Frame, text="BOX 8").grid(column=0, row=7, sticky=(W, E))
        B8_ID_Entry = ttk.Entry(INI_Frame, width=6,textvariable=self.B8_IDStr)
        B8_ID_Entry.grid(column = 1, row=7) 
        B8_Weight_Entry = ttk.Entry(INI_Frame, width=6,textvariable=self.B8_Weight)                                                 
        B8_Weight_Entry.grid(column = 2, row=7)
        B8_Ratio = Spinbox(INI_Frame, textvariable = self.B8_FR_value, width = 3, from_=1, to=1000)
        B8_Ratio.grid(column=4, row=7)
        B8_SchedComboBox = ttk.Combobox(INI_Frame, textvariable=self.B8_sched, width = 12)
        B8_SchedComboBox.grid(column=5, row=7)
        B8_SchedComboBox['values'] = self.sched
        B8_SessionLength = Spinbox(INI_Frame, textvariable = self.B8_SessionLength, width = 3, values = [30, 60, 90, 120, 180, 240, 300, 360])
        B8_SessionLength.grid(column=8, row=7)
        B8_SessionLabel_ =  ttk.Label(INI_Frame, text="Min").grid(column=9, row=7, sticky=(W, E))
        B8_PumpTime = Spinbox(INI_Frame, textvariable = str(self.B8_PumpTime), width = 3, from_=300, to=600)
        B8_PumpTime.grid(column=10,row=7)
        B8_CalcPumpTime = Checkbutton(INI_Frame, text = "Calc  ", variable = self.B8_calcPumpTime, \
                onvalue = True, offvalue = False, command=lambda boxIndex = 7: self.calcPumpTime(self.B8_calcPumpTime.get(),boxIndex))
        B8_CalcPumpTime.grid(column=11, row=7)

        tempLabel =  ttk.Label(INI_Frame, text="            RatID     Weight       Ratio   Sched                                          Session        Pump (10mSec)")
        tempLabel.grid(column=0, row=12, columnspan = 12, sticky = (EW))

        self.loadFromINIFile()
        self.readExampleFiles()
        

        #********************* END of GUI_Class __INIT__(self) ********************

    def calcPumpTime(self,checked,boxIndex):
        """
        4 sec * 5 mg/ml * 0.025 ml/sec / 0.333kg = 1.5 mg/kg
        weight(g) * 12.012 = mSec pump time for 1.5 mg/kg
        Pump time is set in 10mSec intervals so weight(g) * 1.2012 yields proper pumptime setting
        """
        if (checked == 0):
            self.PumpTimeList[boxIndex].set(400)
        else:
            t = int(self.WeightList[boxIndex].get() * 1.2012)
            self.PumpTimeList[boxIndex].set(t)
            # print(boxIndex, "weight = ", self.WeightList[boxIndex].get()," pumptime =", t)           

    def loadFromINIFile(self):
        """
        Reads 8 lines from SA200.ini, parses the line (depending on the position of spaces)
        into "" which are assigned to Parameters in each Box.
        Lists as used to iterate over each variable.
        """       
        if (self.verbose):
            print("Reading SA200.ini")
        iniFile = open('SA200.ini','r')
        for i in range(8):
            aLine = iniFile.readline().rstrip("\n")   # read line 
            tokens = aLine.split()
            if (self.verbose):
                print(tokens)
            self.IDStrList[i].set(tokens[0])
            self.WeightList[i].set(tokens[1])
            # print(self.sched[int(tokens[2])])
            self.schedList[i].set(self.sched[int(tokens[2])])
            self.FR_valueList[i].set(tokens[3])
            self.SessionLengthList[i].set(tokens[4])
            self.PumpTimeList[i].set(tokens[5])
            self.calcPumpTimeList[i].set(tokens[6])
        aString = iniFile.readline().rstrip("\n")      # COM number (done differently on a Mac)
        self.portString.set(aString)
        aLine = iniFile.readline().rstrip("\n")   # read next line
        tokens = aLine.split()
        self.TwoLeverCheckVar.set(tokens[0])
        iniFile.close()

    def writeToINIFile(self):
        INIfileName = "SA200.ini"
        if (self.verbose):
            print("Writing", INIfileName)
        iniFile = open(INIfileName,'w')        
        for i in range(8):
            schedIndex = self.sched.index(self.schedList[i].get())
            IDStr = self.IDStrList[i].get()
            if (len(IDStr) == 0): IDStr = 'box'+str(i+1)
            tempStr = IDStr+' '+ \
            str(self.WeightList[i].get())+' '+ \
            str(schedIndex)+' '+ \
            str(self.FR_valueList[i].get())+' '+ \
            str(self.SessionLengthList[i].get())+' '+ \
            str(self.PumpTimeList[i].get())
            if (self.calcPumpTimeList[i].get() == True):
                tempStr = tempStr+' 1'
            else:
                tempStr = tempStr+' 0'
            if (self.verbose):
                print(tempStr)
            iniFile.write(tempStr+'\n')
        iniFile.write(self.portString.get()+'\n')
        if (self.TwoLeverCheckVar.get() == True):
            iniFile.write("1"+'\n')
        else:
            iniFile.write("0"+'\n')
        iniFile.close()
        
    def readExampleFiles(self):
        self.example1List = []
        aFile = open('PR_Example_File.dat','r')
        for line in  aFile:
            pair = line.split()
            pair[0] = int(pair[0])
            self.example1List.append(pair)
        aFile.close()
        self.example2List = []
        aFile = open('IntA_Example_File.dat','r')
        for line in  aFile:
            pair = line.split()
            pair[0] = int(pair[0])
            self.example2List.append(pair)
        aFile.close()
    
    def saveFile(self,subject_ID,dataList):        
        today = date.today()
        ID_Date = subject_ID+"_"+today.strftime("%b_%d")
        extension = ".dat"
        fileName = ID_Date+extension
        while os.path.isfile(fileName) == True:
            ID_Date = ID_Date+"a"
            fileName = ID_Date+extension
        print("Saving file to", fileName)
        fileToSave = open(fileName,'w')
        for pairs in dataList:
            line = str(pairs[0])+" "+pairs[1]+"\n"
            fileToSave.write(line)
        fileToSave.close()

    def saveAllData(self):
        print("Trying to save data")
        if len(self.box1.dataList) > 0:
            self.saveFile(self.box1.subject_ID_string,self.box1.dataList)
        if len(self.box2.dataList) > 0:
            self.saveFile(self.box2.subject_ID_string,self.box2.dataList)
        if len(self.box3.dataList) > 0:
            self.saveFile(self.box3.subject_ID_string,self.box3.dataList)
        if len(self.box4.dataList) > 0:
            self.saveFile(self.box4.subject_ID_string,self.box4.dataList)
        if len(self.box5.dataList) > 0:
            self.saveFile(self.box5.subject_ID_string,self.box5.dataList)
        if len(self.box6.dataList) > 0:
            self.saveFile(self.box6.subject_ID_string,self.box6.dataList)
        if len(self.box7.dataList) > 0:
            self.saveFile(self.box7.subject_ID_string,self.box7.dataList)
        if len(self.box8.dataList) > 0:
            self.saveFile(self.box8.subject_ID_string,self.box8.dataList)           

    def dumpData(self):
        for pairs in self.boxes[self.selectedBox.get()].dataList:
            line = str(pairs[0])+" "+pairs[1]
            print(line)

    def drawAllTimeStamps(self,aCanvas,selectedList, max_x_scale):
        if (selectedList < 8): dataList = self.boxes[self.selectedBox.get()].dataList
        elif (selectedList == 8): dataList = self.example1List
        elif (selectedList == 9): dataList = self.example2List
        aCanvas.delete('all')
        # max_x_scale = self.topMax_x_Scale.get()
        if max_x_scale == 360: x_divisions = 12
        elif max_x_scale == 180: x_divisions = 6
        elif max_x_scale == 60: x_divisions = 6
        elif max_x_scale == 30: x_divisions = 6
        elif max_x_scale == 10: x_divisions = 10
        x_zero = 65
        y_zero = self.Y_ZERO
        x_pixel_width = 500
        GraphLib.drawXaxis(aCanvas, x_zero, y_zero, x_pixel_width, max_x_scale, x_divisions)
        GraphLib.eventRecord(aCanvas, x_zero, y_zero-250, x_pixel_width, max_x_scale, dataList, ["L"], "L1")
        GraphLib.eventRecord(aCanvas, x_zero, y_zero-220, x_pixel_width, max_x_scale, dataList, ["J"], "L2")
        GraphLib.eventRecord(aCanvas, x_zero, y_zero-190, x_pixel_width, max_x_scale, dataList, ["P","p"], "Pump")
        GraphLib.eventRecord(aCanvas, x_zero, y_zero-160, x_pixel_width, max_x_scale, dataList, ["S","s"], "Stim")
        GraphLib.eventRecord(aCanvas, x_zero, y_zero-130, x_pixel_width, max_x_scale, dataList, ["=","-"], "Lever 1")
        GraphLib.eventRecord(aCanvas, x_zero, y_zero-100, x_pixel_width, max_x_scale, dataList, ["~",","], "Lever 2")
        GraphLib.eventRecord(aCanvas, x_zero, y_zero-70,  x_pixel_width, max_x_scale, dataList, ["t","T"], "Timeout")
        GraphLib.eventRecord(aCanvas, x_zero, y_zero-40,  x_pixel_width, max_x_scale, dataList, ["B","b"], "Block")
        GraphLib.eventRecord(aCanvas, x_zero, y_zero-10,  x_pixel_width, max_x_scale, dataList, ["G","E"], "Session")
    
    def testFunction(self):
        # report maxDelta (uSec)        
        self.outputText("<M>")

    def mimicL1Response(self,boxIndex):
        tempStr = "<L1 "+str(boxIndex)+">"
        self.outputText(tempStr)

    def toggleCheckLevers(self,checkLeversState):
        if (checkLeversState): self.outputText("<CL>")
        else: self.outputText("<cl>")

    def testFunction4(self):
        self.outputText("<B 0>")    # get Block Time

    def testFunction5(self):
        self.outputText("<B 1>")

    def testFunction6(self):        # Get Diagnostics
        self.outputText("<D>")
        self.outputText("<R 7>")
        self.outputText("<R 6>")
        self.outputText("<R 5>")
        self.outputText("<R 4>")
        self.outputText("<R 3>")
        self.outputText("<R 2>")
        self.outputText("<R 1>")
        self.outputText("<R 0>")
        self.outputText("<V>")
        

    def drawCumulativeRecord(self, aCanvas, selectedList, max_x_scale):
        aCanvas.delete('all')
        x_zero = self.X_ZERO
        y_zero = self.Y_ZERO
        x_pixel_width = 500                               
        y_pixel_height = 230
        # max_x_scale = 120
        x_divisions = 12
        if (max_x_scale == 10) or (max_x_scale == 30): x_divisions = 10
        max_y_scale = 100
        y_divisions = 10
        if (selectedList < 8):
            dataList = self.boxes[self.selectedBox.get()].dataList
            aTitle = self.boxes[self.selectedBox.get()].subject_ID_string
        elif (selectedList == 8):
            dataList = self.example1List
            aTitle = "PR example"
        elif (selectedList == 9):
            dataList = self.example2List
            aTitle = "IntA example"
        GraphLib.drawXaxis(aCanvas, x_zero, y_zero, x_pixel_width, max_x_scale, x_divisions)
        GraphLib.drawYaxis(aCanvas, x_zero, y_zero, y_pixel_height, max_y_scale, y_divisions, True)
        GraphLib.cumRecord(aCanvas, x_zero, y_zero, x_pixel_width, y_pixel_height, max_x_scale, max_y_scale, dataList, True, aTitle)
        
    def drawEventRecords(self):
        x_zero = self.X_ZERO
        y_zero = self.Y_ZERO
        x_pixel_width = 500
        max_x_scale = self.selectMax_x_Scale.get()
        x_divisions = 6
        self.topCanvas.delete('all')
        self.topCanvas.create_text(150, 10, fill="blue", text="topCanvas")
        GraphLib.drawXaxis(self.topCanvas, x_zero, y_zero, x_pixel_width, max_x_scale, x_divisions)
        #                eventRecord(aCanvas, x_zero, y_zero, x_pixel_width, max_x_scale, dataList, charList, aLabel):
        GraphLib.eventRecord(self.topCanvas, x_zero, y_zero-225, x_pixel_width, max_x_scale, self.box1.dataList, ["P"], "Box 1")
        GraphLib.eventRecord(self.topCanvas, x_zero, y_zero-200, x_pixel_width, max_x_scale, self.box2.dataList, ["P"], "Box 2")
        GraphLib.eventRecord(self.topCanvas, x_zero, y_zero-175, x_pixel_width, max_x_scale, self.box3.dataList, ["P"], "Box 3")
        GraphLib.eventRecord(self.topCanvas, x_zero, y_zero-150, x_pixel_width, max_x_scale, self.box4.dataList, ["P"], "Box 4")
        GraphLib.eventRecord(self.topCanvas, x_zero, y_zero-125, x_pixel_width, max_x_scale, self.box5.dataList, ["P"], "Box 5")
        GraphLib.eventRecord(self.topCanvas, x_zero, y_zero-100, x_pixel_width, max_x_scale, self.box6.dataList, ["P"], "Box 6")
        GraphLib.eventRecord(self.topCanvas, x_zero, y_zero -75, x_pixel_width, max_x_scale, self.box7.dataList, ["P"], "Box 7")
        GraphLib.eventRecord(self.topCanvas, x_zero, y_zero -50, x_pixel_width, max_x_scale, self.box8.dataList, ["P"], "Box 8")

    def clearCanvas(self, aCanvas):
        if (aCanvas == 0):
            self.topCanvas.delete('all')
        else:
            self.bottomCanvas.delete('all')

        GraphLib.drawXaxis
        

        #+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    def moveLever1(self,boxIndex,state):
        if (state == True): tempStr = "<= "+str(boxIndex)+">"
        else: tempStr = "<- "+str(boxIndex)+">"
        self.outputText(tempStr)

    def moveLever2(self,boxIndex,state):
        if (state == True): tempStr = "<~ "+str(boxIndex)+">"
        else: tempStr = "<, "+str(boxIndex)+">"
        self.outputText(tempStr)

    def togglePump(self,boxIndex,state):
        if (state == True): tempStr = "<P "+str(boxIndex)+">"
        else: tempStr = "<p "+str(boxIndex)+">"
        self.outputText(tempStr)

    def toggleLED1(self,boxIndex,state):
        if (state == True): tempStr = "<S "+str(boxIndex)+">"
        else: tempStr = "<s "+str(boxIndex)+">"
        self.outputText(tempStr)

    def toggleLED2(self,boxIndex,state):
        if (state == True): tempStr = "<C "+str(boxIndex)+">"
        else: tempStr = "<c "+str(boxIndex)+">"
        self.outputText(tempStr)
            
    def connect(self):
        self.portList = []
        self.nt_portList = []
        self.portList = []
        if (os.name == "posix"):
            print("OS = posix (Mac)")
            self.OS_String.set("OS = posix (Mac)")
            self.portList = glob.glob('/dev/tty.usbmodem*')
            print("comports =", self.portList)
            if (len(self.portList) > 0):
                print("found ", self.portList[0])
                self.arduino0.connect(self.portList[0])
                tempStr = "Mac OS found "+str(len(self.portList))+" arduino"
                self.writeToTextbox(tempStr,0)                 
            else:
                self.writeToTextbox("No Feather found",0)
                
        elif(os.name == "nt"):
            self.OS_String.set("OS = Windows")
            self.nt_portList = []
            for i in range(256):
               try:
                   s = serial.Serial(i)
                   self.nt_portList.append(s.portstr)
                   s.close()
               except serial.SerialException:
                   pass
            print("nt_portList =",self.nt_portList)
            self.arduino0.connect(self.portString.get())
            
        if self.arduino0.activeConnection == True:
                self.connectLabelText.set("Connected")
                self.writeToTextbox("Connected!",0)
                time.sleep(2)
                # Request version number from sketch name and version Feather M0
                self.outputText("<V>")
        else:
            self.writeToTextbox("Unable to connect",0)
            self.connectLabelText.set("Unable to connect")
                     
    def startSession(self,boxIndex):
        if self.arduino0.activeConnection == True:
            if self.TwoLeverCheckVar.get() == True:
                self.outputText("<T>")
            else:
                self.outputText("<t>")
            listIndex = boxIndex
            # initialize the box
            self.boxes[listIndex].initialize(self.IDStrList[listIndex].get(),self.WeightList[listIndex].get(),self.schedList[listIndex].get(),1.5)
            # clear any responses and infusion counts if any
            self.L1ResponsesList[listIndex].set(0)
            self.L2ResponsesList[listIndex].set(0)
            self.InfList[listIndex].set(0)
            # send Parameters 
            self.outputText("<SCHED "+str(boxIndex)+" "+str(self.schedList[listIndex].get())+">")
            self.outputText("<RATIO "+str(boxIndex)+" "+str(self.FR_valueList[listIndex].get())+">")
            self.outputText("<TIME "+str(boxIndex)+" "+str(self.SessionLengthList[listIndex].get()*60)+">")
            self.outputText("<PUMP "+str(boxIndex)+" "+str(self.PumpTimeList[listIndex].get())+">")
            # request report
            self.outputText("<R "+str(boxIndex)+">")
            # send Go signal
            self.outputText("<G "+str(boxIndex)+">")
        else:
           self.writeToTextbox("No arduino connected",0)

    def stopSession(self, boxIndex):
        self.outputText("<Q "+str(boxIndex)+">")


    def startAllBoxes(self):
        self.writeToINIFile()
        for i in range(8):
            self.startSession(i)
            sleep(0.1) # Time in seconds.

    def stopAllBoxes(self):
        for i in range(8):
            self.stopSession(i)
            # sleep(0.25) # Time in seconds.

    def reportPhase(self,arduino):
        print("Report Phase for ardunino number",arduino)
        """
        self.outputText("<r>")
            { Serial.print(millis());
              if (phase == 0) Serial.println(" Prestart");
              if (phase == 1) Serial.println(" timein");
              if (phase == 2) Serial.println(" timeout");
              if (phase == 3) Serial.println(" finished");
        """

    def toggleEchoInput(self):
        self.outputText("<E>")       

    def reportPortStatus(self):

        if self.arduino0.activeConnection == True:
            self.writeToTextbox("arduino connected",0)
        else:
            self.writeToTextbox("arduino not connected", 0)          
                       
        print(self.arduino0.serialPort)
        print("Input",self.arduino0.inputThread)
        print("Output",self.arduino0.outputThread)
        print("input Queue", self.arduino0.inputQ)
        print("Output Queue", self.arduino0.outputQ)

    def updateTimer(self,sessionTime,listIndex):
        #sessionTime = (inputTokens[2])
        h = str(sessionTime // 3600)
        m = str((sessionTime % 3600) // 60)
        if (len(m) < 2):
            m = "0"+m
        s = str((sessionTime % 60))
        if (len(s) < 2):
            s = "0"+s
        self.sessionTimeStrList[listIndex].set(h+":"+m+":"+s)

    def handleInput(self, inputLine):
        """
        Document this!
        """
        boxNum = 99
        strCode = "XX"
        timeStamp = 0
        level = 0
        boolVarListIndex = -1
        if (self.showDataStreamCheckVar.get() == 1):
            self.writeToTextbox(inputLine,0)
            # print(inputLine)
        tokens = inputLine.split()
        # print(inputLine, "length = ", len(tokens))
        try:
            if len(tokens) > 1:
                #print(tokens[0],tokens[1])
                boxNum = (int(tokens[0].decode()))
                strCode = tokens[1].decode()   # convert from "byte literal"
            if len(tokens) > 2:
                timeStamp = int(tokens[2].decode())
            if len(tokens) > 3:
                level = (int(tokens[3].decode()))
                boolVarListIndex = (int(tokens[4].decode()))
        except:
            print('exception at getInput: ',tokens)   
        if (boxNum in [0,1,2,3,4,5,6,7]):
            listIndex = boxNum           
            if (strCode == "*"):    # timers get updated but nothing gets added to datafile
                self.updateTimer(timeStamp,listIndex)
            else:                   # every other timestamp gets added to the datafile
                self.boxes[listIndex].dataList.append([timeStamp, strCode])     # append timestamp to dataList - eventually datafile
                if (boolVarListIndex >= 0):
                    self.boxesVarLists[listIndex][boolVarListIndex].set(level)           # update checkbox                   
                if (strCode == "L"):
                    self.L1ResponsesList[listIndex].set(self.L1ResponsesList[listIndex].get()+1)    # update response label
                if (strCode == "J"):
                    self.L2ResponsesList[listIndex].set(self.L2ResponsesList[listIndex].get()+1)    # update Dummy response label
                elif (strCode == "P"):
                    self.InfList[listIndex].set(self.InfList[listIndex].get()+1)                # update response label
        elif (boxNum == 9): self.writeToTextbox(strCode,0)


    def pumpChaos(self):
        self.pumpChaosCount = self.pumpChaosCount + 1
        if self.pumpChaosCount >= self.pumpChaosDelay:
            self.pumpChaosCount = 0
            if self.pumpChaosState == True:
                tempStr = "<P "+str(self.pumpChaosBoxNum)+">"
            else:
                tempStr = "<p "+str(self.pumpChaosBoxNum)+">"
                #print(tempStr)
            self.outputText(tempStr)
            self.pumpChaosBoxNum = self.pumpChaosBoxNum + 1
            if self.pumpChaosBoxNum > 7:
                self.pumpChaosBoxNum = 0
                self.pumpChaosState = not(self.pumpChaosState)       

    def periodic_check(self):
        if self.arduino0.activeConnection == True:    
            while not self.arduino0.inputQ.empty():
                try:
                    inputLine = self.arduino0.getInput()
                    self.handleInput(inputLine)
                    if self.pumpChaosCheckVar.get() == 1:
                        self.pumpChaos()
                except queue.Empty:
                    pass
        self.root.after(100, self.periodic_check)  # procedure reschedules its own reoccurance in 100 mSec

    def writeToTextbox(self,text,aTextBox):
        if aTextBox == 0:
            self.topTextbox.insert('1.0',text)
            self.topTextbox.insert('1.0',"\n")
        else:
            self.bottomTextbox.insert('1.0',text)
            self.bottomTextbox.insert('1.0',"\n")
        # print(text)

    def outputText(self,text):
        self.arduino0.outputQ.put(text)
        if self.showOutputCheckVar.get() == 1:
            self.writeToTextbox(text,1)
    
    def about(self):
        tkMessageBox.showinfo("About",__doc__,parent=self.root)
        return

    def go(self):
        self.root.after(100, self.periodic_check)
        self.root.mainloop()

class Box(object):
    def __init__(self,boxNum):
        """
        This function essentially creates a new box. Initial values are arbitrary
        The initialize procedure sets (or resets) the values after the box has been created. 
        """
        self.boxNum = boxNum        
        self.subject_ID_string = 'box'+str(boxNum)
        self.weight = 330
        self.schedNum = 0      # To Do: define schedules such as 0 = "do not run", 1 = FR etc.
        self.dose = 1.0
        self.dataList = []

    def initialize(self,ID_Str, initWeight, initSched, initDose):
        self.subject_ID_string = ID_Str
        self.weight = initWeight
        self.schedNum = initSched
        self.dose = initDose
        self.dataList = []
        self.report()

        """
        timestamp = int(str(event)[1:str(event).find(',')])
        char = str(event)[-3]
        data.append([timestamp, char])

        """

    def report(self):
        print("Initialize report: ",self.subject_ID_string,self.weight,self.schedNum,self.dose)



if __name__ == "__main__":
    sys.exit(main())  
    # exit(main())  see  http://stackoverflow.com/questions/6501121/the-difference-between-exit-and-sys-exit-in-python
