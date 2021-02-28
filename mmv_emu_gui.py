import subprocess
import tkinter as tk
import threading


class display():

	def __init__(self, master):

		frame = tk.Frame(master)
		frame.pack()

		self.seven_seg = []

		self.seven_seg.append(self.create_7segment(frame, True, False))
		self.seven_seg.append(self.create_7segment(frame, True, True))
		self.seven_seg.append(self.create_7segment(frame, True, False))
		self.seven_seg.append(self.create_7segment(frame, False, False))


	def create_7segment(self, master, create_dot, create_colon):

		canvas = tk.Canvas(master, width = 140, height = 190, bg = 'black', highlightthickness = 0)
		canvas.pack(side = 'left')

		canvas.create_polygon(93, 18, 81, 30, 33, 30, 21, 18, 33, 6, 81, 6,	width = 0)
		canvas.create_polygon(96, 21, 108, 33, 108, 81, 96, 93, 84, 81, 84, 33, width = 0)
		canvas.create_polygon(96, 99, 108, 111, 108, 159, 96, 171, 84, 159, 84, 111, width = 0)
		canvas.create_polygon(93, 174, 81, 186, 33, 186, 21, 174, 33, 162, 81, 162, width = 0)
		canvas.create_polygon(18, 99, 30, 111, 30, 159, 18, 171, 6, 159, 6, 111, width = 0)
		canvas.create_polygon(18, 21, 30, 33, 30, 81, 18, 93, 6, 81, 6, 33, width = 0)
		canvas.create_polygon(93, 96, 81, 108, 33, 108, 21, 96, 33, 84, 81, 84,	width = 0)

		if create_dot:
			canvas.create_rectangle(118, 166, 135, 186, width = 0)

		if create_colon:
			canvas.create_rectangle(118, 166-105, 135, 186-105, width = 0, tag = 'col')
			canvas.create_rectangle(118, 166-55, 135, 186-55, width = 0, tag = 'col')

		return canvas


	def update(self, data):

		for i, d  in enumerate(data):
			for x in range(8):
				self.update_segment(i, x+1, d & (1<<x))

		self.update_segment(1, 'col', data[3] & 128)


	def update_segment(self, idx, tag, active):

		if active:
			self.seven_seg[idx].itemconfigure(tag, fill = 'red')
		else:
			self.seven_seg[idx].itemconfigure(tag, fill = 'black')


class led_bar():

	def __init__(self, master):

		frame = tk.Frame(master)
		frame.pack()

		self.leds = []

		for i in range(6):
			led = tk.Canvas(frame, width = 10, height = 10, bg = 'black')
			led.grid(row = 0, column = i*2)
			self.leds.append(led)

		tk.Label(frame, text = 'PLAY').grid(row = 0, column = 1)
		tk.Label(frame, text = 'POS').grid(row = 0, column = 3)
		tk.Label(frame, text = 'MEM').grid(row = 0, column = 5)
		tk.Label(frame, text = 'INFO').grid(row = 0, column = 7)
		tk.Label(frame, text = 'WHITE').grid(row = 0, column = 9)
		tk.Label(frame, text = 'BLACK').grid(row = 0, column =11)


	def update(self, flags):

		for i in range(6):
			if flags & (1<<i):
				self.leds[5-i]['bg'] = 'red'
			else:
				self.leds[5-i]['bg'] = 'black'


class keyboard():

	def __init__(self, master, out_stream):

		self.out_stream = out_stream

		w = 5
		f = ('bold', '20')

		kb = tk.Frame(master)
		kb.pack()

		tk.Button(kb, text = 'CL',   width = w, font = f, command = lambda: self.key_cb('CL')  ).grid(row = 0, column = 0)
		tk.Button(kb, text = 'POS',  width = w, font = f, command = lambda: self.key_cb('POS') ).grid(row = 0, column = 2)
		tk.Button(kb, text = 'MEM',  width = w, font = f, command = lambda: self.key_cb('MEM') ).grid(row = 0, column = 3)
		tk.Button(kb, text = 'INFO', width = w, font = f, command = lambda: self.key_cb('INFO')).grid(row = 0, column = 4)
		tk.Button(kb, text = 'LEV',  width = w, font = f, command = lambda: self.key_cb('LEV') ).grid(row = 0, column = 5)
		tk.Button(kb, text = 'ENT',  width = w, font = f, command = lambda: self.key_cb('ENT') ).grid(row = 0, column = 6)

		tk.Button(kb, text = 'A1 ♙', width = w, font = f, command = lambda: self.key_cb('A1')).grid(row = 1, column = 0)
		tk.Button(kb, text = 'B2 ♘', width = w, font = f, command = lambda: self.key_cb('B2')).grid(row = 1, column = 2)
		tk.Button(kb, text = 'C3 ♗', width = w, font = f, command = lambda: self.key_cb('C3')).grid(row = 1, column = 3)
		tk.Button(kb, text = 'D4 ♖', width = w, font = f, command = lambda: self.key_cb('D4')).grid(row = 1, column = 4)
		tk.Button(kb, text = 'E5 ♕', width = w, font = f, command = lambda: self.key_cb('E5')).grid(row = 1, column = 5)
		tk.Button(kb, text = 'F6 ♔', width = w, font = f, command = lambda: self.key_cb('F6')).grid(row = 1, column = 6)

		tk.Button(kb, text = 'RES', width = w, font = f, state = 'disabled').grid(row = 2, column = 0)
		tk.Button(kb, text = 'RES', width = w, font = f, state = 'disabled').grid(row = 2, column = 2)

		tk.Button(kb, text = 'G7',  width = w, font = f, command = lambda: self.key_cb('G7')   ).grid(row = 2, column = 3)
		tk.Button(kb, text = 'H8',  width = w, font = f, command = lambda: self.key_cb('H8')   ).grid(row = 2, column = 4)
		tk.Button(kb, text = '< ⬛', width = w, font = f, command = lambda: self.key_cb('LEFT') ).grid(row = 2, column = 5)
		tk.Button(kb, text = '> ⬜', width = w, font = f, command = lambda: self.key_cb('RIGHT')).grid(row = 2, column = 6)


	def key_cb(self, key):

		self.out_stream.write('KEY_' + key + '\n')
		self.out_stream.flush()


def inp_thread(inp_stream):

	while True:
		inp = inp_stream.readline()
		print(inp, end = '')

		if len(inp) >= 14:

			disp1 = int(inp[0:2], 16)
			disp2 = int(inp[3:5], 16)
			disp3 = int(inp[6:8], 16)
			disp4 = int(inp[9:11], 16)
			disp.update((disp1, disp2, disp3, disp4))

			flags = int(inp[13:15], 16)
			leds.update(flags)


# main
mmv = subprocess.Popen(['ya65c02'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)

root = tk.Tk()
root.title('MMV Emulator')
root.resizable(False, False)

leds = led_bar(root)
disp = display(root)
kb = keyboard(root, mmv.stdin)

threading.Thread(target = inp_thread, args = (mmv.stdout,), daemon = True).start()

root.mainloop()

mmv.terminate()