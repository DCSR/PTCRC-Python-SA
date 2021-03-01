
import math
"""
GraphLib.py is used both by SelfAdminXXX.py and AnalysisBeta.py
SelfAdmin defines two 600x300 canvases
X_ZERO = 50
Y_ZERO = 275

AnalysisBeta defines two 800x600 canvases


def cumRecord(aCanvas, dataList, max_x_scale, max_y_scale):
def cumRecord(aCanvas, dataList, max_x_scale, x_divisions, x_pixel_width,
                                  max_y_scale, y_divisions, y_pixel_height):

XlogScaler = 3.5
YlogScaler = 2
"""

def drawXaxis(aCanvas, x_zero, y_zero, x_pixel_width, max_x_scale, x_divisions, color = "black"):
    """
    Draws an X (horizontal) axis using the following parameters:
    aCanvas, x_zero, y_zero, x_pixel_width, max_x_scale, x_divisions
    """
    # aCanvas.create_text(300, 20, text="graphLibTest")
    aCanvas.create_line(x_zero, y_zero, x_zero + x_pixel_width, y_zero, fill=color)
    for divisions in range(x_divisions + 1):          
        x = x_zero + (divisions * (x_pixel_width // x_divisions))
        aCanvas.create_line(x, y_zero, x, y_zero + 5, fill=color)
        aCanvas.create_text(x, y_zero + 20, text=str(int((max_x_scale/x_divisions)*divisions)), fill=color)
        
def drawLogXAxis(aCanvas,x_zero,y_zero,x_pixel_width,xLogScaler):
        aCanvas.create_text(350, y_zero+50, fill="blue", text="Price (responses/mg cocaine)")
        x_scaler = x_pixel_width / xLogScaler
        aCanvas.create_line(x_zero, y_zero, x_zero + x_pixel_width, y_zero, fill="blue")
        logScale = [1,2,3,4,5,6,7,8,9,10,20,30,40,50,60,70,80,90,100,200,300,400,500,600,700,800,900,1000]
        for i in logScale:
            x = (x_zero + ((math.log(i,10))* x_scaler)) // 1
            aCanvas.create_line(x, y_zero, x, y_zero + 5,fill="blue")
        LabelList = [1,30,10,300,100]
        for i in LabelList:
            x = (x_zero + (((math.log(i,10))* x_scaler))) // 1
            aCanvas.create_text(x, y_zero+20, fill="blue", text=str(i))

def drawYaxis(aCanvas, x_zero, y_zero, y_pixel_height, max_y_scale, y_divisions, labelLeft, color = "black"):
    """
    Draws an Y (verticle) axis using the following parameters:
    (aCanvas, x_zero, y_zero, y_pixel_height, max_y_scale, y_divisions,  labelLeft, color):

    labelLeft = True places labels and tick marks on the left of the axis
    Adjust x_zero to move the axis left or right.
        eg. x_zero = self.X_ZERO + x_pixel_width will push it all the way to the right edge. 
    """
    aCanvas.create_line(x_zero, y_zero, x_zero, y_zero-y_pixel_height, fill=color)
    for divisions in range(y_divisions+1):          
        y = y_zero - (divisions * (y_pixel_height // y_divisions))
        if labelLeft:        # create_text and hash marks on left side of the axis 
            aCanvas.create_line(x_zero, y, x_zero-5, y, fill=color)
            aCanvas.create_text(x_zero -20, y, fill = color, text=str(int((max_y_scale / y_divisions)*divisions)))
        else:
            aCanvas.create_line(x_zero, y, x_zero+5, y, fill=color)
            aCanvas.create_text(x_zero + 20, y, fill = color, text=str((max_y_scale // y_divisions)*divisions))

def drawLogYAxis(aCanvas,x_zero,y_zero,y_pixel_height,yLogScaler):
        # aCanvas.create_text(x_zero, y_zero-200, fill="blue", text="Consumption")
        # ToDo: add a arg or kwarg for Axis Label
        y_scaler = y_pixel_height / yLogScaler        
        aCanvas.create_line(x_zero, y_zero, x_zero, y_zero-y_pixel_height, fill="blue")
        logScale = [0.01,0.02,0.03,0.04,0.05,0.06,0.07,0.08,0.09,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1,2]
        for i in logScale:
            y = ((y_zero-(math.log(100,10)* y_scaler)) - ((math.log(i,10))* y_scaler)) // 1
            aCanvas.create_line(x_zero, y, x_zero+5, y,fill="blue")
        LabelList = [0.01,0.03,0.1,0.3,1,10]
        for i in LabelList:
            y = ((y_zero-(math.log(100,10)* y_scaler)) - ((math.log(i,10))* y_scaler)) // 1
            aCanvas.create_text(x_zero+20, y, fill="blue", text=str(i))

def plotResponseCurve(aCanvas,x_zero,y_zero,x_pixel_width,y_pixel_height,maxYScale,priceList,responseList):
        """
        Draws a black line with black symbols using responseList
        which is composed of pairs: price and total responses in each block
        """
        # maxYScale = threshRecYMax.get() 
        resp_x = x_zero-25
        resp_y = y_zero
        y_scaler = (y_pixel_height / maxYScale)
        x_scaler = x_pixel_width / self.XlogScaler
        for t in range(12):    # 0..11
            price = xList[t]
            new_x = (self.x_zero + (math.log(price,10)*x_scaler)) // 1
            new_y = (self.y_zero - (yList[t] * y_scaler)) // 1
            symbol = canvas.create_oval(new_x-4,new_y-4,new_x+4,new_y+4,fill="black")
            if t > 0:
                canvas.create_line(resp_x, resp_y, new_x, new_y,fill="black")
            resp_x = new_x
            resp_y = new_y




def logLogPlot(aCanvas,x_zero,y_zero,x_pixel_width,y_pixel_height, \
               xLogScaler,yLogScaler,xList,yList,drawSymbol,drawLine,color):
        """
        Draws a log-log plot using a list of x and y values.
        Parmeters:
        aCanvas : 

        x, new_x, y and new_y refer to pixel values on the canvas
        x_value and y_value refer data points to be plotted  
        """
        # print(xList)
        # print(yList)       
        x_scaler = x_pixel_width / xLogScaler
        y_scaler = y_pixel_height / yLogScaler       
        x = x_zero-25
        y = y_zero
        for t in range(len(xList)):    # 0..11
            x_value = xList[t]
            new_x = (x_zero + (math.log(x_value,10)*x_scaler)) // 1
            y_value = yList[t]
            # print(x_value,y_value)
            if y_value > 0.01:
                new_y = ((y_zero-(math.log(100,10)* y_scaler)) - ((math.log(y_value,10))* y_scaler)) // 1
            else:
                new_y = y_zero
            # new_y = (y_zero - (pairs[1] * y_scaler)) // 1
            if drawSymbol:
                symbol = aCanvas.create_oval(new_x-4,new_y-4,new_x+4,new_y+4, fill=color)
            if drawLine:
                if t > 0:
                    aCanvas.create_line(x,y,new_x,new_y,fill=color)
            x = new_x
            y = new_y
            
def eventRecord(aCanvas, x_zero, y_zero, x_pixel_width, max_x_scale, dataList, charList, aLabel, t_zero = 0):
    """
    t_zero is the millis() at start time. Most timestamps are saved as session time, but error timetsamps
    are saved as raw millis() and therefore need to have session start time subtracted
    """
    x = x_zero
    y = y_zero
    x_scaler = x_pixel_width / (max_x_scale*60*1000)
    aCanvas.create_text(x_zero-65, y_zero-5, fill="blue", text = aLabel, anchor = "w") 
    for pairs in dataList:
        if pairs[1] in charList:
            newX = (x_zero + ((pairs[0]-t_zero) * x_scaler) // 1)
            aCanvas.create_line(x, y, newX, y)
            if (len(charList) == 1):           #eg. charList = ["P"]
                aCanvas.create_line(newX, y, newX, y-10)
            else:                              #eg. charlist = ["B","b"]
                if pairs[1] == charList[0]:
                    newY = y_zero -10
                else:
                    newY = y_zero                
                aCanvas.create_line(newX, y, newX, newY)
                y = newY                        
            x = newX
            # aCanvas.create_text(x, y_zero+10, fill="blue", text = pairs[1])  # show char underneath 

def cumRecord(aCanvas, x_zero, y_zero, x_pixel_width, y_pixel_height, max_x_scale, max_y_scale, dataList, showBP, aTitle):
    aCanvas.create_text(100, 20, fill="blue", text = aTitle)
    lastX = x_zero
    lastY = y_zero
    resets = 0
    x_scaler = x_pixel_width / (max_x_scale*60*1000)
    y_scaler = y_pixel_height / max_y_scale
    respTotal = 0
    # stuff for calculating breakpoint
    BP_not_found = True
    pumpStartTime = 0
    lastTime = 0
    BP_time = 0
    numberOfInfusions = 0
    breakpoint = 0
    delta = 0
    BP_X = x_zero
    BP_Y = y_zero
    for pairs in dataList:
        if pairs[1] == 'L':
            respTotal = respTotal + 1
            adjustedRespTotal = respTotal - (resets * max_y_scale)
            newX = x_zero + (pairs[0] * x_scaler) // 1
            aCanvas.create_line(lastX, lastY, newX, lastY)
            newY = y_zero - (adjustedRespTotal * y_scaler // 1)
            if newY < (y_zero - y_pixel_height):
                aCanvas.create_line(newX, lastY, newX, (y_zero-y_pixel_height))    #Line to max
                aCanvas.create_line(newX, y_zero-y_pixel_height, newX, y_zero)   #line to Y_ZERO
                resets = resets + 1
                adjustedRespTotal = respTotal - (resets * max_y_scale)
                newY = y_zero - (adjustedRespTotal * y_scaler // 1)
            aCanvas.create_line(newX, lastY, newX, newY)
            lastX = newX
            lastY = newY
        if pairs[1] == 'P':
                aCanvas.create_line(lastX, lastY, lastX+5, lastY+5)
                # Look for breakpoint:
                numberOfInfusions = numberOfInfusions + 1
                pumpStartTime = pairs[0]
                delta = round((pumpStartTime - lastTime)/(1000*60))   # delta in minutes
                if (delta < 60) and BP_not_found:   
                    breakpoint = breakpoint + 1
                    BP_X = lastX
                    BP_Y = lastY
                else:
                    BP_not_found = False
                lastTime = pumpStartTime
    if showBP: 
        aCanvas.create_text(300, 20, fill="red", text = "Infusions = "+str(numberOfInfusions))
        aCanvas.create_text(500, 20, fill="red", text = "breakpoint = "+str(breakpoint))
        aCanvas.create_oval(BP_X-8,BP_Y-8,BP_X+8,BP_Y+8, outline = "red")

    
    
    

    
