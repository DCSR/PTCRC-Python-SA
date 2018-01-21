

from tkinter import *
import tkinter.ttk as ttk
import tkinter.scrolledtext
from datetime import datetime, date, time
from tkinter import messagebox
from os.path import expanduser
import os



def main(argv=None):
    if argv is None:
        argv = sys.argv
    gui = GuiClass()
    gui.go()
    return 0


class GuiClass(object):
    def __init__(self):

        self.root = Tk()
        self.root.protocol("WM_DELETE_WINDOW", self.askBeforeExiting)      
        self.root.title("tkinter test")
        self.sessionRunning = False
        self.dataSaved = True  # Initially, there's none to save
        self.startTime = 0
        self.endTime = 0

        # Left Top Frame
        self.Frame1 = ttk.Frame(self.root,borderwidth=5, relief="sunken")
        self.Frame1.grid(column = 0, row = 0)

        Label1 = ttk.Label(self.Frame1, text="Session Frame")
        Label1.grid(column = 0, row = 0)
        
        self.timeLabel = StringVar()
        Label5 = ttk.Label(self.Frame1, textvariable = self.timeLabel)
        Label5.grid(column = 0, row = 1)
        self.timeLabel.set("0:00:00")

        self.startButton = ttk.Button(self.Frame1,text="Start Sesssion",command=self.startSession)
        self.startButton.grid(column = 0, row = 2)

        self.stopButton = ttk.Button(self.Frame1,text="Stop Session",command=self.endSession)
        self.stopButton.grid(column = 0, row = 3)

        self.Button1 = ttk.Button(self.Frame1,text="B1 Time Stamp",command=lambda arg = 1: self.buttonPress(arg))
        self.Button1.grid(column = 0, row = 4)

        self.Button8 = ttk.Button(self.Frame1,text="B8 Time Stamp",command=lambda arg = 8: self.buttonPress(arg))
        self.Button8.grid(column = 0, row = 5)

        self.saveDataButton = ttk.Button(self.Frame1,text="Save Data",command=lambda arg = 8: self.buttonPress(arg))
        self.saveDataButton.grid(column = 0, row = 6) 

        # Left Middle Frame
        self.Frame2 = ttk.Frame(self.root,borderwidth=5, relief="sunken")
        self.Frame2.grid(column = 0, row = 1)

        Label2 = ttk.Label(self.Frame2, text="Messagebox Examples")
        Label2.grid(column = 0, row = 0)

        self.Button2 = ttk.Button(self.Frame2,text="Show Info",command=self.spawnShowInfoMessagebox)
        self.Button2.grid(column = 0, row = 1)

        self.Button3 = ttk.Button(self.Frame2,text="Question", command = self.spawnQuestionMessagebox)
        self.Button3.grid(column = 0, row = 2)

        self.Button4 = ttk.Button(self.Frame2,text="Yes or No?", command = self.spawnAskYesNoMessagebox)
        self.Button4.grid(column = 0, row = 3)

        self.Button5 = ttk.Button(self.Frame2,text="Retry?", command = self.spawnAskRetryCancelMessagebox)
        self.Button5.grid(column = 0, row = 4)

        self.Button6 = ttk.Button(self.Frame2,text="Error!", command = self.spawnShowErrorMessagebox)
        self.Button6.grid(column = 0, row = 5)

        self.Button7 = ttk.Button(self.Frame2,text="Warning", command = self.spawnShowWarningMessagebox)
        self.Button7.grid(column = 0, row = 6)        

        # Test Frame
        self.Frame3 = ttk.Frame(self.root,borderwidth=5, relief="sunken")
        self.Frame3.grid(column = 0, row = 2)

        Label3 = ttk.Label(self.Frame3, text="Test Frame")
        Label3.grid(column = 0, row = 0, columnspan = 2)

        self.pathButton = ttk.Button(self.Frame3,text="Path info", command = self.test)
        self.pathButton.grid(column = 0, row = 1, columnspan = 2)       

        self.directoryButton = ttk.Button(self.Frame3,text="Directory listing", command = self.dirList)
        self.directoryButton.grid(column = 0, row = 2, columnspan = 2)

        self.createFolderButton = ttk.Button(self.Frame3,text="Create Folder", command = self.createFolder)
        self.createFolderButton.grid(column = 0, row = 3, columnspan = 2)

        self.testString = StringVar(value="Enter")
        TestEntry = ttk.Entry(self.Frame3, width=6,textvariable=self.testString)
        TestEntry.grid(column = 0, row = 4)

        Label4 = ttk.Label(self.Frame3, textvariable=self.testString)
        Label4.grid(column = 1, row = 4)
        
        # Text Frame
        self.Frame5 = ttk.Frame(self.root,borderwidth=5, relief="sunken")
        self.Frame5.grid(column = 1, row = 0, rowspan = 3)

        self.textBox = tkinter.scrolledtext.ScrolledText(self.Frame5,height=20,width= 75)
        self.textBox.grid(column = 0, row = 0)
        self.textBox.insert('1.0', 'Top row\n')

    def startSession(self):
        self.startTime = datetime.now()
        tempStr = self.startTime.strftime("%B %d at %H:%M:%S") 
        self.textBox.insert('1.0', 'Session Started on '+tempStr+'\n')
        self.dataSaved = False
        self.sessionRunning = True

    def endSession(self):
        self.endTime = datetime.now()
        tempStr = self.endTime.strftime("%B %d at %H:%M:%S") 
        self.textBox.insert('1.0', 'Session Ended on '+tempStr+'\n')
        self.sessionRunning = False

    def spawnShowInfoMessagebox(self):
        messagebox.showinfo("ShowInfo title", \
            "This is an info messagebox\n\n Note that the clock stops\nwhen any dialog box is open")

    def spawnQuestionMessagebox(self):
        myVar = messagebox.askquestion("Question title", "True ot False?")
        # print(myVar)
        self.textBox.insert('1.0', "Answer = "+str(myVar)+"\n")

    def spawnAskYesNoMessagebox(self):
        myVar = messagebox.askyesno("Yes-No title", "Yes or No?")
        # print(myVar)
        self.textBox.insert('1.0', "Answer = "+str(myVar)+"\n")

    def spawnAskRetryCancelMessagebox(self):
        myVar = messagebox.askretrycancel("Retry title", "Retry message")
        # print(myVar)
        self.textBox.insert('1.0', "Retry = "+str(myVar)+"\n")

    def spawnShowErrorMessagebox(self):
        myVar = messagebox.showerror("Error title", "ERROR!!!")
        # print(myVar)
        self.textBox.insert('1.0', "Response to message = "+str(myVar)+"\n")

    def spawnShowWarningMessagebox(self):
        myVar = messagebox.showerror("Warning title", "Warning! Be careful!")
        # print(myVar)
        self.textBox.insert('1.0', "Response = "+str(myVar)+"\n")

    def test(self):
        self.textBox.delete("1.0",END)
        # ----  Detect Operating System ----------
        osName = os.name
        if (os.name == "posix"):
            self.textBox.insert(END, "Operating system detected: 'posix' (Mac)\n")
        elif(os.name == "nt"):
            self.textBox.insert(END, "Operating system detected: 'nt' (Windows)\n")          
        cwd = os.getcwd()
        self.textBox.insert(END, "Current Working Directory = \n"+cwd+"\n")
        dataDir = date.today().strftime("%b_%d_%Y")
        if os.path.exists(dataDir):
            self.textBox.insert(END, "Data Directory /"+dataDir+" exists \n")
        else:
            self.textBox.insert(END, "Data Directory /"+dataDir+" does not exist \n")
            
               

    def dirList(self):
        myFiles = os.listdir()
        #os.listdir() returns a list of files in directory - seems to include subdirectory names 
        self.textBox.insert(END, "Directory = \n")
        count = 0
        fileNum = 0
        for files in myFiles:
            count = count + 1
            self.textBox.insert(END, files+"      ")
            fileNum = fileNum + 1
            if (fileNum == 2):
                fileNum = 0
                self.textBox.insert(END, files+"\n")
        self.textBox.insert(END,"\n")               
        self.textBox.insert(END, "Number of Files = "+str(count)+"\n")

    def createFolder(self):
        dataDir = date.today().strftime("%b_%d_%Y")
        if not os.path.exists(dataDir):
            myVar = messagebox.askyesno("/"+dataDir+"/", \
                    "/"+dataDir+" does not exist \n\n Create it?")
            if (myVar == True):
                print("Creating "+dataDir)
                os.makedirs(dataDir)
                if os.path.exists(dataDir):
                    messagebox.showinfo("Info", "/"+dataDir+" now exists")               
        else:
            messagebox.showinfo(dataDir, dataDir+" exists")                                            

    def saveFile(self):
        self.textBox.insert('1.0', "Saving Data\n")
        self.dataSaved = True

    def buttonPress(self, num):
        time = datetime.now()
        tempStr = "Button "+str(num)+" at "+str(time.strftime("%H:%M:%S")+"\n")
        self.textBox.insert('1.0', tempStr)

    def askBeforeExiting(self):
        if (self.sessionRunning == True):
            messagebox.showerror("Stop Session Warning", \
                                 "Stop Session before closing program")
        elif (self.dataSaved == False):
            response = messagebox.askyesno("Save Data?", "Save Data before exiting?")
            if (response == True):
                self.saveFile()
                self.root.destroy()
            else:
                self.root.destroy()
        else:
            self.root.destroy()

    def periodic_check(self):
        # http://docs.python.org/dev/library/datetime.html#strftime-strptime-behavior
        """
        Update the clock while session is running
        """
        if (self.sessionRunning == True):
            now = datetime.now()
            tdelta = now - self.startTime
            seconds = int(tdelta.total_seconds())
            h = str(seconds // 3600)
            m = str((seconds % 3600) // 60)
            if (len(m) < 2):
                m = "0"+m
            s = str((seconds % 60))
            if (len(s) < 2):
                s = "0"+s
            self.timeLabel.set(h+":"+m+":"+s)
        self.root.after(100, self.periodic_check)

    def go(self):
        self.root.after(100, self.periodic_check)
        self.root.mainloop()


if __name__ == "__main__":
    sys.exit(main())  
