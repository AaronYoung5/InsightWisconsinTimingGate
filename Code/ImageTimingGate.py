import cv2 as cv
import time as t

def image_manipulation(img):
	gray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
	for i in range(10):
		gaussian = cv.GaussianBlur(gray, (21,21),0)
	return gaussian

def get_threshold(firstFrame, frame, low=25):
	frameDelta = cv.absdiff(firstFrame, frame)
	thresh = cv.threshold(frameDelta, low, 255, cv.THRESH_BINARY)[1]

	return thresh

def write_centerline(img):
	height, width = img.shape[:2]
	cv.line(img, (width/2,0), (width/2,height), (0, 255, 255), thickness=2, lineType=8, shift=0)	

def find_contours(img,min=20000):
	# dilate the thresholded image to fill in holes, then find contours
	# on thresholded image
	thresh = cv.dilate(img, None, iterations=5)
	image, cnts, hierarchy = cv.findContours(thresh.copy(), cv.RETR_EXTERNAL,
		cv.CHAIN_APPROX_SIMPLE)
 	
	img = cv.cvtColor(img, cv.COLOR_GRAY2BGR)
		
	# loop over the contours
	for c in cnts:
		in_center(c,img)
		# if the contour is too small, ignore it
		if cv.contourArea(c) < min:
			continue
 
		# compute the bounding box for the contour, draw it on the frame,
		# and update the text
		(x, y, w, h) = cv.boundingRect(c)
		cv.rectangle(img, (x, y), (x + w, y + h), (0, 255, 0), 2)
	return img
	
def in_center(c,img):
	height, width = img.shape[:2]
	(x, y, w, h) = cv.boundingRect(c)
	if int(x + w/2) == int(width/2):
		print('Half Way')

cam = cv.VideoCapture(0)

success = True

firstFrame = None

time = float(0)

while(success):
	success, frame = cam.read()
	frame = image_manipulation(frame)
	if firstFrame is None:
		firstFrame = frame
		continue
	#img = cv.cvtColor(get_threshold(firstFrame, frame), cv.COLOR_GRAY2RGB)
	img = get_threshold(firstFrame, frame)
	img = find_contours(img,min=(img.shape[:2][1]*img.shape[:2][0])/2)
	write_centerline(img)	
	cv.imshow('Thresh',img)
	firstFrame = frame
	k = cv.waitKey(1)
	if k == ord('q'):
		break
	elif k == 32:
		time = t.time()
#		print(time)
	elif k == 116:
		print(t.time() - time)



cam.release()
cv.destroyAllWindows()

