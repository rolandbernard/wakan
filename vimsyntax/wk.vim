" Vim syntax file
" Language: Wakan 0.1
" Maintainer: Roland Bernard
" Latest Revision: 26.12.2018

if exists("b:current_syntax")
  finish
endif

syn keyword wkTodo contained TODO FIXME XXX NOTE
syn match wkComment "#.\{-}$" contains=wkTodo
syn match wkComment "#.\{-}#" contains=wkTodo

syn keyword wkCmd sin cos tan asin acos atan
syn keyword wkCmd sinh cosh tanh asinh acosh atanh
syn keyword wkCmd floor ceil trunc local global
syn keyword wkCmd to_str to_num to_ascii to_bool
syn keyword wkCmd def write len
syn keyword wkCmd struct dic find

syn keyword wkVar rand read self func_self

syn keyword wkOpr and or xor not mod

syn keyword wkControl for in do while if then else

syn match wkOp ":="
syn match wkOp "+"
syn match wkOp "-"
syn match wkOp "\*"
syn match wkOp "/"
syn match wkOp ":"
syn match wkOP "<"
syn match wkOp ">"
syn match wkOp "<="
syn match wkOp ">="
syn match wkOp "="
syn match wkOp "->"
syn match wkOp "\."

syn match wkSep ","
syn match wkSep ";"
syn match wkSep "\\"

syn match wkNumber "\d\+\(\.\d*\)\=\(e[+-]\=\d*\)\="

syn match wkNormalChar "\zs\\\ze[^0abtnvfr]\|\\\n" contained

syn match wkSpecialChar "\\[0abtnvfr]" contained

syn region wkString start='"' end='"' contains=wkNormalChar,wkSpecialChar
syn region wkString start='\''  end='\'' contains=wkNormalChar,wkSpecialChar
syn region wkList start='\[' end='\]' fold transparent
syn region wkScope start='{' end='}' fold transparent
syn region wkBrac start='(' end=')' fold transparent
syn region wkAbs start='|' end='|' fold transparent

let b:current_syntax = "wk"

hi def link wkTodo Todo
hi def link wkComment Comment
hi def link wkNumber Constant
hi def link wkString Constant
hi def link wkCmd Function
" hi def link wkFunc Function " couldn't find good regex
hi def link wkControl Statement
" hi def link wkOp Operator " to much
hi def link wkOpr Operator
hi def link wkNormalChar Comment
hi def link wkSpecialChar SpecialChar
hi def link wkVar Macro
