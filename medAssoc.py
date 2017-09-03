
'''
    B, b = Start and stop of a first lever block
    A, a = Start and stop of a second lever block
    L, l = First lever down and up
    J, j = Second lever down and up
    =, . = First lever extend (=) and retract (.)
    -, , = Second lever extend (-) and retract (,)
    T, t = Start and stop of a trial
    S, s = Stimulus light on and off
    P, p = Pump on and off
    E    = End of session
    R    = Restart session
'''


def read_Samaha_file(fileName):
    ''' (string) -> list of [int, char]

    filepath is a string containing a valid filepath to a .txt data file.
    Returns a list of lists with each sublist containing an integer
    representing the timestamp and a char representing the type of event.
    
    Format from Anna Noel Samaha's lab


    A variable : Total number of infusions during the 6-H session
    E variable : Total number of inactive-lever presses during the 6-H session
    D and Q variables (The same - because FR1) : Number of infusions inside each 6-minutes active phases
    F variable : Number of inactive lever presses inside each 6-minutes active phase
    K variable : Difference between infusion times (If you need more details about the interval between infusions, i have studied extensively this parameter inside each  6-minutes active phases) 
    S variable (the more important) : The time where infusions have been consumed during the session. 
    

    '''

    #---------------------------------
    # print("Opening Samaha file: ", fileName)
    
    dataList = []

    #respList = [0,0,0,0,0,0,0,0,0,0,0,0]
    #pumpList = [0,0,0,0,0,0,0,0,0,0,0,0]
    #pumpTimes = [8175, 4597, 2585, 1454, 818, 460, 259, 145, 82, 46, 26, 0]
    #MUSC pump times - could be read from the file
    infusions = 0
    responses = 0
    duration = 5000
    pumpKeyFound = False
    lastPumptimeFound = False
    with open(fileName) as myOpenFile:
        for line in myOpenFile:
            """
            aFile = open(fileName,'r')
            line = aFile.readline()
            print("First line: ",line)
            while line != "":
                line = aFile.readline()
                print(line)
            """
            if pumpKeyFound and not(lastPumptimeFound):
                if len(line) > 0:
                    if line[0] == " ":
                        tokens = line.split()
                        for token in tokens:
                            if token.find(".") > 0:
                                if token == "0.000":
                                    lastPumptimeFound = True
                                else:
                                    infusions = infusions + 1
                                    # print(token)
                                    time = int((float(token)*1000)//1)
                                    pair = [time,"P"]
                                    dataList.append(pair)
                                    pair = [time,"L"]
                                    dataList.append(pair)
                                    pair = [time+duration,"p"]
                                    dataList.append(pair)
                    else:
                        pumpKeyFound = False                
            if line == "S:\n":
                pumpKeyFound = True
        # print("infusions: ", infusions)
        myOpenFile.close()
        for i in range(12):   # Add "start block" char at 10 min intervals
            blockStartTime = 1000 * 60 * 30 * i
            pair = [blockStartTime,"B"]  # (mSec* 60 sec * 30 min)       
            dataList.append(pair)
            pair = [blockStartTime+(1000*60*6),"b"]  # plus 6 min
            dataList.append(pair)

    sortedList = sorted(dataList)
    return sortedList


def read_txt_file(fileName):
    ''' (string) -> list of [int, char]

    filepath is a string containing a valid filepath to a .txt data file.
    Returns a list of lists with each sublist containing an integer
    representing the timestamp and a char representing the type of event.
    
    '''

    #---------------------------------
    print(fileName)
    dataList = []
    respList = [0,0,0,0,0,0,0,0,0,0,0,0]
    pumpList = [0,0,0,0,0,0,0,0,0,0,0,0]
    pumpTimes = [8175, 4597, 2585, 1454, 818, 460, 259, 145, 82, 46, 26, 0]
    #MUSC pump times - could be read from the file
    lineNumber = 0
    infusions = 0
    responses = 0
    leverKeyFound = False
    pumpKeyFound = False
    with open(fileName) as myOpenFile:
        for line in myOpenFile:
            """
            aFile = open(fileName,'r')
            line = aFile.readline()
            print("First line: ",line)
            while line != "":
                line = aFile.readline()
                print(line)
            """
            lineNumber = lineNumber + 1
            if pumpKeyFound:
                if len(line) > 0:
                    if line[0] == " ":
                        tokens = line.split()
                        for token in tokens:
                            if token.find(".") > 0:
                                infusions = infusions + 1
                                time = int((float(token)*1000)//1)
                                pair = [time,"P"]
                                dataList.append(pair)
                                timeBin = time//600000
                                duration = pumpTimes[timeBin]
                                pair = [time+duration,"p"]
                                dataList.append(pair)
                                pumpList[timeBin] = pumpList[timeBin] + duration
                    else:
                        pumpKeyFound = False                
            if leverKeyFound:
                if len(line) > 0:
                    if line[0] == " ":
                        #print(line.rstrip()) 
                        tokens = line.split()
                        for token in tokens:
                            if token.find(".") > 0:
                                responses = responses + 1
                                time = int((float(token)*1000)//1)
                                pair = [time,"L"]
                                dataList.append(pair)
                                timeBin = time//600000
                                respList[timeBin] = respList[timeBin] + 1
                    else:
                        leverKeyFound = False
            if line == "R:\n":
                # print("R: found at line number", lineNumber)
                leverKeyFound = True
            if line == "Q:\n":
                # print("Q: found at line number", lineNumber)
                pumpKeyFound = True
        myOpenFile.close()
        for i in range(12):   # Add "start block" char at 10 min intervals
            pair = [(600000 * i),"B"]        
            dataList.append(pair)

        """
        print(dataList)
        print("Med Assoc total responses: ", responses)
        print("Med Assoc infusions: ", infusions)
        print("Med Assoc respList: ",respList)
        print("Med Assoc pumpList: ",pumpList)
        """
        consumptionList = [0,0,0,0,0,0,0,0,0,0,0,0]
        for i in range(12):
            mgCoc = pumpList[i] * 0.00006825  # duration * (2.73 mg/ml * 0.000025 mls/mSec)
            consumptionList[i] = mgCoc
        print(consumptionList)
        sortedList = sorted(dataList)
        return sortedList
