(begin
	(print (quote ("Scheme.C4 Repl")))

	(define repl (prompt) (begin
		(print prompt (quote ">"))
		(repl-handle (repl-get))
	))

	(define repl-get (lambda () (begin
		(quote (print (+ 1 2))))))
