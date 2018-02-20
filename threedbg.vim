py3 << EOF
import threedbg
import numpy as np
import io
import vim
import atexit
atexit.register(threedbg.free)
EOF

function! s:Init()
    py3 threedbg.init()
endfunction

function! s:Free()
    py3 threedbg.free()
endfunction

function! s:Toggle()
    py3 << EOF
if threedbg.working():
    threedbg.free()
else:
    threedbg.init()
EOF
endfunction

function! s:ClearPoint()
    py3 threedbg.Point.clear()
    py3 threedbg.Point.flush()
endfunction

function! s:ClearLine()
    py3 threedbg.Line.clear()
    py3 threedbg.Line.flush()
endfunction

function! s:ReadData(l1, l2)
    execute 'py3 start = ' . (a:l1 - 1)
    execute 'py3 end   = ' . a:l2
    py3 << EOF
content = "\n".join(vim.current.buffer[start:end+1]) + "\n"
del start, end
data = np.loadtxt(io.StringIO(content), dtype=np.float32)
del content
EOF
" now `data` stores the array
endfunction

function! s:DelData()
    py3 del data
endfunction

function! s:AddPoint(l1, l2)
    call s:ReadData(a:l1, a:l2)
    py3 << EOF
threedbg.Point.add(data[:,:3])
threedbg.Point.flush()
EOF
    call s:DelData()
endfunction

function! s:AddPointWithColor(l1, l2)
    call s:ReadData(a:l1, a:l2)
    py3 << EOF
threedbg.Point.add(data[:, :3], data[:,3:6])
threedbg.Point.flush()
EOF
    call s:DelData()
endfunction

function! s:AddLine(l1, l2)
    call s:ReadData(a:l1, a:l2)
    py3 << EOF
threedbg.Line.add(data[:, :3], data[:,3:6])
threedbg.Line.flush()
EOF
    call s:DelData()
endfunction

command! TDBInit call s:Init()
command! TDBFree call s:Free()
command! TDBToggle call s:Toggle()

command! TDBPointClear call s:ClearPoint()
command! TDBLineClear call s:ClearLine()
command! TDBClear call s:ClearPoint() | call s:ClearLine()

" x x x
command! -range=% TDBAddPoint call s:AddPoint(<line1>, <line2>)
" x x x  x x x
command! -range=% TDBAddPointWithColor call s:AddPointWithColor(<line1>, <line2>)
" x x x  x x x
command! -range=% TDBAddLine call s:AddLine(<line1>, <line2>)
