class Display:	                            # class names are by convention capitalized
    def __init__(self, name, canvas):
        self.name = name               # stuff inside the object
        self.x_zero = 100
        self.y_zero = 450		       # create and initialize other stuff
        self.canvas_height = 500
        self.canvas_width = 800
        self.Y_pixel_height = 400
        self.X_pixel_width = 600
        self.max_X_scale = 60
        self.max_Y_scale = 500
        self.X_divisions = 12
        self.Y_divisions = 10
        self.canvas = canvas

    def __str__(self):
        """ returns a string value of values inside object
        """
        s = self.selectedFileName
        return s

    def cumRec(self, selectedList):
        # my_bubble = canvas.create_oval(200,200,210,210)
        print('CumRec')




"""
eg.

canvasDisplay = Dislay("DisplayName", 100,450)


"""
