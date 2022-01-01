#!/usr/bin/env python3
usage='''                                                          Hot keys:
The script can be run in terminal as                             q: Quit
    ./rosa.py                                                    h: Help
or                                                               i: Run idx.C
    python3 rosa.py                                              b: Run b2r.C
It can also be launched with double clicks in a file browser.    v: Run view.C

# List boxes #

- "Select experiment:" click to select a folder to
  - show the daq.cfg file in that folder, and
  - list index files in that folder if there is any.
- "Select index file:" click to select an index file to show its content.
- "Select ROOT file:" click to select a ROOT file to show its size and creation date.

# Buttons #

- "Run idx.C": click to pop a window to select a binary file to parse.
- "Run b2r.C": click to convert a binary file to ROOT file.
- "Run view.C": click to launch a ROOT TTreeViewer if ROOT is installed.
'''
from tkinter import *
root=Tk(); root.resizable(0,0)
root.wm_title('Convert Struck DAQ binary output to ROOT trees' +
       ' (press q to quit, h for help)')
def quit_gui(event=None): root.quit(); root.destroy()
root.bind('q', quit_gui)

Label(root, text="Select experiment:").grid(column=0, row=0, sticky='nw')
elist=Listbox(root, height=8,
# https://anzeljg.github.io/rin2/book2/2405/docs/tkinter/listbox.html
        selectbackground='orchid', selectforeground='white',
# https://stackoverflow.com/a/48992537/1801749
        exportselection=False)
elist.grid(column=0, row=1, sticky='ew'); elist.focus()

folders=[] # obtain a list of folders containing daq.cfg
from os import walk, system, path, listdir
for folder, subdirs, files in walk('.'):
    if '.git' in subdirs: subdirs.remove('.git')
    if 'daq.cfg' in files: folders.append(folder)
folders.sort()
for folder in folders:
    elist.insert("end",folder)
    if elist.size()%2: elist.itemconfig("end", bg='azure', fg='black')

Label(root, text="Select index file:").grid(column=1, row=0, sticky='nw')
ilist=Listbox(root,height=8,exportselection=False,
        selectbackground='orchid',selectforeground='white')
ilist.grid(column=1, row=1, sticky='ew')

Label(root, text="Select ROOT file:").grid(column=2, row=0, sticky='nw')
rlist=Listbox(root, height=8)
rlist.grid(column=2, row=1, sticky='ew')

from subprocess import Popen
def run_idx_C(event=None):
    print(elist.curselection()[0])
    folder=elist.get(elist.curselection()[0]).replace('\\','/')
index=Button(root, text='Run idx.C', state='disabled', command=run_idx_C)
index.grid(column=0, row=2, sticky='se')
index.bind('<Return>', run_idx_C)
root.bind('i', run_idx_C)

def run_b2r_C(event=None):
    print(ilist.curselection()[0])
    folder=elist.get(elist.curselection()[0]).replace('\\','/')
    file=ilist.get(ilist.curselection()[0]).replace('\\','/')
    file=folder+'/'+file
    Popen(['root', '-l', 'b2r.C("'+file+'")'])
conv=Button(root, text='run b2r.C', state='disabled', command=run_b2r_C)
conv.grid(column=2, row=2, sticky="se")
conv.bind('<Return>', run_b2r_C)
root.bind('v', run_b2r_C)

def run_view_C(event=None):
    print(elist.curselection()[0])
    folder=elist.get(elist.curselection()[0]).replace('\\','/')
    file=rlist.get(rlist.curselection()[0]).replace('\\','/')
    file=folder+'/'+file
    Popen(['root', '-l', 'view.C("'+file+'")'])
view=Button(root, text='run view.C', state='disabled', command=run_view_C)
view.grid(column=2, row=2, sticky="se")
view.bind('<Return>', run_view_C)
root.bind('v', run_view_C)

text=Text(root, width=120, height=25)
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
    index['state']='normal' if rlist.size()>0 else 'disabled'
    view['state']='normal' if rlist.size()>0 else 'disabled'

def experiment_selected(event=None):
    folder=elist.get(elist.curselection()[0]).replace('\\','/')
    list_files_in(folder)
    text.delete(1.0,'end'); text.config(wrap=WORD)
    with open(folder+'/daq.cfg', 'r') as f: text.insert(INSERT, f.read())
elist.bind("<<ListboxSelect>>", experiment_selected)

def index_folder_selected(event=None):
    text.delete(1.0,'end'); text.config(wrap=NONE)
    folder=elist.get(elist.curselection()[0])
    file=ilist.get(ilist.curselection()[0])
    with open(folder+'/'+file, 'r') as f: text.insert(INSERT, f.read())
ilist.bind("<<ListboxSelect>>", index_folder_selected)

# give focus to the GUI window in Mac
# https://stackoverflow.com/questions/17774859
from platform import system as platform
if platform() == 'Darwin':  # How Mac OS X is identified by Python
    system('''/usr/bin/osascript -e 'tell app "Finder" to set frontmost of process "Python" to true' ''')

# If you put root.destroy() here, it will cause an error if the window is
# closed with the window manager.
root.mainloop()
