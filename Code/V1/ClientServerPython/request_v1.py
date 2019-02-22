# importing the requests library 
import requests 
import datetime
from pandas import DataFrame
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from tkinter import *

class GUI:
	def __init__(self,root):
		self.root = root
		#self.time = StringVar()
		#self.frame = Frame(self.root)
		#self.frame.pack()
		#self.label = Label(frame, textvariable=self.time)
		#self.label.grid(row=0, column=1, sticky=W)
		#self.label.pack()
		#self.update('test')

	def update(self,text):
		self.time.set(text)
		#self.root.mainloop()

class Plot:
	def __init__(self):
		self.left,self.width = .25, .5
		self.bottom, self.height = .25, .5
		self.hor_center = self.left + self.width/2
		self.vert_center = self.bottom + self.height/2
		fig = plt.figure()
		self.ax = fig.add_axes([0,0,1,1])
		self.p = patches.Rectangle((self.left, self.bottom), self.width, self.height, fill=False, clip_on=False,transform=self.ax.transAxes)
		self.ax.add_patch(self.p)	
	
	def update(self,text):
		self.ax.text(self.vert_center, self.hor_center, text, horizontalalignment='center', verticalalignment='center', transform=self.ax.transAxes)
		self.ax.set_axis_off()
		plt.show(block=False)

class Saver:

	def __init__(self):
		self.clientsPassed = [False,False]
		self.clientsIP = ['192.168.4.16', '192.168.4.17']
		#self.file = open('data%s.txt' % datetime.datetime.now().time(), 'a') 
		self.file = open('recorded_times.txt', 'w')
		self.clientsTime = [0.0,0.0]


	def save_response(self,response):
		response_arr = response.split(' ')
		self.file.write(response + '\n')

	def compute_time(self,response):
		self.file = open('recorded_times.txt', 'a')
		response_arr = response.split(' ')
		if self.clientsIP[0] == response_arr[1] and not self.clientsPassed[0]:
			self.clientsTime[0] = float(response_arr[0])
			self.clientsPassed[0] = True

		elif self.clientsIP[1] == response_arr[1] and not self.clientsPassed[1]:
			self.clientsTime[1] = float(response_arr[0])
			self.clientsPassed[1] = True

		if self.clientsPassed[0] and self.clientsPassed[1]:
			print(abs(self.clientsTime[0] - self.clientsTime[1]))
			self.file.write(str(abs(self.clientsTime[0] - self.clientsTime[1])))
			self.file.write('\n') 
			self.file.close()
			self.clientsPassed = [False,False]

saver = Saver()
#plot = Plot()
win = Tk()
#gui = GUI(root)

#GET Response from IP
while True:
	response = requests.get("http://192.168.4.15/Python")

	if response.text != 'No New Data':
		print(response.text)
		plot.update(response.text)
		saver.compute_time(response.text)
