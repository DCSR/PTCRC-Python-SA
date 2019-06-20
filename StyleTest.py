
"""

Docs: https://tkdocs.com/tutorial/styles.html

Specifying the proportion of red, green and blue in hexadecimal digits.
For example, "#fff" is white, "#000000" is black, "#000fff000" is pure green,
and "#00ffff" is pure cyan (green plus blue).

Locally defined standard color names are "white", "black", "red", "green",
"blue", "cyan", "yellow", and "magenta" will always be available.

tkinter and ttk have what appear to be identical widgets but they are configured differently.
tkinter have more defined options but ttk widgets are more easily configured with "Styles".

Note the following differences between Frame1 and Frame2 (tkinter and ttk respectively)

print("tkinter.Frame",self.Frame1.config())
tkinter.Frame {'bd': ('bd', '-borderwidth'), 'borderwidth': ('borderwidth', 'borderWidth', 'BorderWidth',
<pixel object: '0'>, 5), 'class': ('class', 'class', 'Class', 'Frame', 'Frame'), 'relief': ('relief', 'relief',
'Relief', <index object: 'flat'>, 'sunken'), 'background': ('background', 'background', 'Background', <border object:
'systemWindowBody'>, 'systemWindowBody'), 'bg': ('bg', '-background'), 'colormap': ('colormap', 'colormap',
'Colormap', '', ''), 'container': ('container', 'container', 'Container', 0, 0), 'cursor': ('cursor', 'cursor',
'Cursor', '', ''), 'height': ('height', 'height', 'Height', <pixel object: '0'>, 0), 'highlightbackground':
('highlightbackground', 'highlightBackground', 'HighlightBackground', <color object: 'systemWindowBody'>, 'blue'),
'highlightcolor': ('highlightcolor', 'highlightColor', 'HighlightColor', <color object: 'Black'>, 'blue'),
'highlightthickness': ('highlightthickness', 'highlightThickness', 'HighlightThickness', <pixel object: '0'>, 0),
'padx': ('padx', 'padX', 'Pad', <pixel object: '0'>, <pixel object: '0'>), 'pady': ('pady', 'padY', 'Pad',
<pixel object: '0'>, <pixel object: '0'>), 'takefocus': ('takefocus', 'takeFocus', 'TakeFocus', '0', '0'),
'visual': ('visual', 'visual', 'Visual', '', ''), 'width': ('width', 'width', 'Width', <pixel object: '0'>, 0)}
**********
print("tkinter.Frame",self.Frame2.config())
ttk.Frame {'borderwidth': ('borderwidth', 'borderWidth', 'BorderWidth', '', 5), 'padding': ('padding', 'padding',
'Pad', '', ''), 'relief': ('relief', 'relief', 'Relief', '', <index object: 'sunken'>), 'width': ('width', 'width',
'Width', <pixel object: '0'>, <pixel object: '0'>), 'height': ('height', 'height', 'Height', <pixel object: '0'>,
<pixel object: '0'>), 'takefocus': ('takefocus', 'takeFocus', 'TakeFocus', '', ''), 'cursor': ('cursor', 'cursor',
'Cursor', '', ''), 'style': ('style', 'style', 'Style', '', ''), 'class': ('class', '', '', '', '')}
**********
Note that highlightbackground="blue" can be included in the definition of the tkinter instantiation of Frame1 but
it throws an error if included in the instantiation of Frame2.

print("Theme names", self.style.theme_names()) - > ('aqua', 'clam', 'alt', 'default', 'classic')

print("Button Class", self.Button1.winfo_class())  -> TButton
print("Frame Class", self.Frame1.winfo_class())    -> Frame
print("Label Class", self.Label1.winfo_class())    -> TLabel

print(self.style.lookup("TButton", "font"))

Here ttk.Style() is experimented with.


https://stackoverflow.com/questions/16639125/how-do-i-change-the-background-of-a-frame-in-tkinter
from tkinter import *
from tkinter.ttk import * 

root = Tk()

s = Style()
s.configure('My.TFrame', background='red')

mail1 = Frame(root, style='My.TFrame')
mail1.place(height=70, width=400, x=83, y=109)
mail1.config()
root.mainloop()




 
"""

from tkinter import *
import tkinter.ttk as ttk
# from tkinter.ttk import *   <- this would cause ttk widgets to automatically replace many standard tk widgets 
import tkinter.scrolledtext
from datetime import datetime, date, time


def main(argv=None):
    if argv is None:
        argv = sys.argv
    gui = GuiClass()
    gui.go()
    return 0

class GuiClass(object):
    def __init__(self):
        self.root = Tk()
        #self.root.geometry("400x300")
        self.root.title("Style test")
        self.root.configure(background='black')

        """
        s=ttk.Style()
        print("Style:",s)
        
        print("Elements:",s.element_names())

        ttk.Style().configure(ttk.Button, padding=6, relief="sunken",
        background="#000000", activeforeground = "#00ffff")

        ttk.Style().configure("TFrame", padding=10, relief="flat",
        background="#000fff000", activeforeground = "#00ffff")   

        # ttk.Style().configure("TFrame", padding=10, relief="flat",
        # background="Black")

        """
        BG_COLOR = 'dark slate gray'
        FG_COLOR = 'white'

        self.style=ttk.Style()
        self.style.theme_use("alt")       
        self.style.configure("TLabel", foreground=FG_COLOR, background=BG_COLOR)
        self.style.configure('TFrame', background=BG_COLOR)
        self.style.configure("TButton", foreground=FG_COLOR, background=BG_COLOR)
        self.style.configure("TEntry", foreground="grey", fieldbackground=BG_COLOR)
        """
        txt.tag_config('warning', background="yellow", foreground="red")

        txt.insert('end', "Hello\n")
        txt.insert('end', "Alert #1\n", 'warning')
        txt.insert('end', "World\n")
        txt.insert('end', "Alert #2\n", 'warning')
        """
        #self.style.configure("TEntry", foreground="white", background="black")
        #self.style.configure("Text", foreground="white", background="black")

        # Top Left Frame
        self.Frame1 = ttk.Frame(self.root,borderwidth=5, relief="sunken")
        self.Frame1.grid(column = 0, row = 0)

        # Top Middle Frame
        self.Frame2 = ttk.Frame(self.root,borderwidth=5, relief="sunken")
        self.Frame2.grid(column = 0, row = 1)

        # Top Right Frame
        self.Frame3 = ttk.Frame(self.root,borderwidth=5, relief="sunken")
        self.Frame3.grid(column = 0, row = 2)

        # Middle Left Frame
        self.Frame4 = ttk.Frame(self.root,borderwidth=5, relief="sunken")
        self.Frame4.grid(column = 0, row = 3, columnspan = 3, sticky = (EW))        

        # Text Frame
        self.Frame5 = ttk.Frame(self.root,borderwidth=5, relief="sunken")
        self.Frame5.grid(column = 1, row = 0, rowspan = 3)

        # Timer Row
        self.Frame6 = ttk.Frame(self.root,borderwidth=5, relief="sunken")
        self.Frame6.grid(column = 0, row = 4, columnspan = 3, sticky = (EW))

        self.Label1 = ttk.Label(self.Frame2, text="  Top  ")
        self.Label1.grid(column = 0, row = 0)

        self.Button1 = ttk.Button(self.Frame1,text="Dark Mode", command=lambda arg = 1: self.switchStyle("alt"))
        self.Button1.grid(column = 0, row = 2)

        Label2 = ttk.Label(self.Frame2, text="Middle",style="DarkMode.TLabel")
        Label2.grid(column = 0, row = 2)

        self.Button2 = ttk.Button(self.Frame2,text="Clam",command=lambda arg = 2: self.switchStyle("clam"))
        self.Button2.grid(column = 0, row = 1)

        Label3 = ttk.Label(self.Frame3, text="Bottom")
        Label3.grid(column = 0, row = 0)

        self.Button3 = ttk.Button(self.Frame3,text="Classic",command=lambda arg = 3: self.switchStyle("classic"))
        self.Button3.grid(column = 0, row = 2) 
        
        self.testString = StringVar(value="Enter")

        Label4 = ttk.Label(self.Frame4, textvariable=self.testString)
        Label4.grid(column = 1, row = 0)

        self.TestEntry = ttk.Entry(self.Frame4, width=6,textvariable=self.testString)
        self.TestEntry.grid(column = 0, row = 0)

        print("Entry Class", self.TestEntry.winfo_class())

        self.TextBox = tkinter.scrolledtext.ScrolledText(self.Frame5,height=10,width= 25,bg='beige')
        self.TextBox.grid(column = 0, row = 0)
        self.TextBox.insert('1.0', 'A text box')

        print("Text Class", self.TextBox.winfo_class())

        self.timeLabel = StringVar()
        Label5 = ttk.Label(self.Frame6, textvariable = self.timeLabel)
        Label5.grid(column = 0, row = 0)

    def switchStyle(self, aStyle):
        self.style.theme_use(aStyle)
        self.TextBox.insert('1.0', aStyle)
        

    def buttonPress(self, num):
        time = datetime.now()
        tempStr = "*" 
        if (num == 1):
            self.style.theme_use("alt")
            tempStr = str(num)+" alt at "+str(time.strftime("%H:%M:%S")+"\n") 
        elif (num == 2):
            self.style.theme_use("clam")
            tempStr = str(num)+" clam at "+str(time.strftime("%H:%M:%S")+"\n") 
        elif (num == 3): self.switchStyle("classic")
        self.TextBox.insert('1.0', tempStr)

    def periodic_check(self):
        # http://docs.python.org/dev/library/datetime.html#strftime-strptime-behavior
        time = datetime.now()
        self.timeLabel.set(time.strftime("%B %d ---- %H:%M:%S"))        
        self.root.after(100, self.periodic_check)

    def go(self):
        self.root.after(100, self.periodic_check)
        self.root.mainloop()


if __name__ == "__main__":
    sys.exit(main())  
