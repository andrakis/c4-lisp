(begin
	(define fac (lambda (n) (fac2 n 1)))
	(define fac2 (lambda (n acc)
		(if (<= n 1) acc (fac2 (- n 1) (* n acc)))))

	(define x 10)

	(print (quote (Factorial of)) x (quote is) (fac x))
)
