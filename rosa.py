#!/usr/bin/env python3
usage='''                                                              Hot keys:
The script can be run in terminal as                                 q: Quit
    ./b2r.py                                                         h: Help
or                                                                   c: Convert
    python3 b2r.py
It can also be launched with double clicks in a file browser.

Click to select a data folder to 
 - list index files in that folder and
 - show the README.md file in that folder
Click to select an index file to show its content.

Click the "Convert" button to run 
    root -l b2r.C'("the selected index file")'
'''
from tkinter import *
root=Tk(); root.resizable(0,0)
root.wm_title('Convert Struck DAQ binary output to ROOT trees' +
       ' (press q to quit, h for help)')
def quit_gui(event=None): root.quit(); root.destroy()
root.bind('q', quit_gui)

Label(root, text="Select data folder:").grid(column=0, row=0, sticky='nw')
dlist=Listbox(root, height=8,
# https://anzeljg.github.io/rin2/book2/2405/docs/tkinter/listbox.html
        selectbackground='orchid', selectforeground='white',
# https://stackoverflow.com/a/48992537/1801749
        exportselection=False)
dlist.grid(column=0, row=1, sticky='ew'); dlist.focus()

folders=[] # obtain a list of folders containing README.md
from os import walk, system, path, listdir
for folder, subdirs, files in walk('.'):
    if '.git' in subdirs: subdirs.remove('.git')
    if 'README.md' in files: folders.append(folder)
folders.remove('.')
folders.sort()
for folder in folders:
    dlist.insert("end",folder)
    if dlist.size()%2: dlist.itemconfig("end", bg='azure', fg='black')
dlist.see(dlist.size()-1) # scroll to the last folder

Label(root, text="Select index file:").grid(column=1, row=0, sticky='nw')
ilist=Listbox(root,height=8,exportselection=False,
        selectbackground='orchid',selectforeground='white')
ilist.grid(column=1, row=1, sticky='ew')

Label(root, text="Select ROOT file:").grid(column=2, row=0, sticky='nw')
rlist=Listbox(root, height=8)
rlist.grid(column=2, row=1, sticky='ew')

from subprocess import Popen
def call_show_py(event=None):
    if dlist.size()==0 or rlist.size()==0: return
    folder=dlist.get(dlist.curselection()[0]).replace('\\','/')
    Popen(['python3', 'show.py', folder])
show=Button(root, text='Show', state='disabled', command=call_show_py)
show.grid(column=2, row=2, sticky='se')
show.bind('<Return>', call_show_py)
root.bind('s', call_show_py)

def call_view_C(event=None):
    if dlist.size()==0 or rlist.size()==0: return
    folder=dlist.get(dlist.curselection()[0]).replace('\\','/')
    Popen(['root', '-l', 'view.C("'+folder+'")'])
view=Button(root, text='View trees', state='disabled', command=call_view_C)
view.grid(column=2, row=2, sticky="sw")
view.bind('<Return>', call_view_C)
root.bind('v', call_view_C)

text=Text(root, width=160, height=25)
text.grid(column=0, row=3, columnspan=3)
def show_usage(event=None):
    text.delete(1.0,'end')
    text.insert(INSERT, usage)
root.bind('h', show_usage);

show_usage()

def list_files_in(folder=''):
    ilist.delete(0,'end'); rlist.delete(0,'end')
    for folders, subdirs, files in walk(folder):
        for file in files:
            if file[-4:]==".csv": ilist.insert("end",file)
            if file[-4:]=="root": rlist.insert("end",file)
        if ilist.size()%2: ilist.itemconfig("end", bg='azure', fg='black')
    conv['state']='normal' if ilist.size()>0 else 'disabled'
    show['state']='normal' if rlist.size()>0 else 'disabled'
    view['state']='normal' if rlist.size()>0 else 'disabled'

def data_folder_selected(event=None):
    folder=dlist.get(dlist.curselection()[0]).replace('\\','/')
    list_files_in(folder)
    text.delete(1.0,'end'); text.config(wrap=WORD)
    with open(folder+'/README.md', 'r') as f: text.insert(INSERT, f.read())
dlist.bind("<<ListboxSelect>>", data_folder_selected)

def index_folder_selected(event=None):
    text.delete(1.0,'end'); text.config(wrap=NONE)
    folder=dlist.get(dlist.curselection()[0])
    file=ilist.get(ilist.curselection()[0])
    with open(folder+'/'+file, 'r') as f: text.insert(INSERT, f.read())
ilist.bind("<<ListboxSelect>>", index_folder_selected)

def convert_file(event=None):
    run=dlist.get(dlist.curselection()[0]).replace("\\","/")
    file=ilist.get(ilist.curselection()[0])
    if file[-3:]=='bin': ch=file[8:9] # CoMPASS output
    else:
        underscore=file.find('_',3,7)
        dot=file.find('.',-4)
        if underscore<0: ch=file[4:dot] # WaveDump output: wave0.dat
        else: ch=file[underscore+1:dot] # WaveDump output: wave_0.dat
    script='c2r.C' if file[-3:]=='bin' else 'w2r.C'
    if file[-3:]=="bin": run=run+"/RAW"
    argument='{}("{}","{}",{},{},{},{},{},{})'.format(
            script,run,file,ch,thr,polarity,nbase,ssize,bits)
    Popen(['root', '-b', '-q', argument]).wait()
    list_files_in(run)

conv=Button(root, text='Convert', state='disabled', command=convert_file)
conv.grid(column=1,row=2)
conv.bind('<Return>', convert_file)
root.bind('c', convert_file)

# give focus to the GUI window in Mac
# https://stackoverflow.com/questions/17774859
from platform import system as platform
if platform() == 'Darwin':  # How Mac OS X is identified by Python
    system('''/usr/bin/osascript -e 'tell app "Finder" to set frontmost of process "Python" to true' ''')

# If you put root.destroy() here, it will cause an error if the window is
# closed with the window manager.
root.mainloop()
