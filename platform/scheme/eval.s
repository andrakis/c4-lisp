(begin
	(quote !Run in a lambda to avoid polluting global env !)

	(define eval ((lambda (x) (begin
		(quote !Main eval entry point !)
		(define eval (lambda (x env) (begin
			(if (null? env) (define env (kernel:get_env/0)))

			(define t (typeof x))
			(if (= t 'symbol) (env^get x env)
				(if (= t 'number) x
					(if (null? x) nil
						(eval-list x env))))
		)))

		(quote !Utility functions !)
		(define nth (lambda (n list) (if (<= 0 n) (head list) (nth (- n 1) (tail list)))))
		(define env^get  (lambda (key env) (kernel:env_get/2 key env)))
		(define env^set  (lambda (key value env) (kernel:env_set/3 key value env)))

		(quote !eval-list: evaluate a list (might be a function call, or a keyword) !)
		(define eval-list (lambda (x env) (begin
			(define h (head x))
			(define t (typeof h))
			((if (= t 'symbol) eval-symbol eval-fun) (eval h env) (tail x) env)
		)))

		(quote !eval-symbol: attempt to evaluate a symbol (for keywords), if no match go to eval-fun !)
		(define eval-symbol (lambda (sym x env)
			(if (= sym 'quote) (head (tail x))
				(if (= sym 'if) (eval-if x env)
					(if (= sym 'set) (eval-set x env)
						(if (= sym 'define) (env^set (head x) (head (tail x)) env)
							(if (= sym 'lambda) (eval-lambda x env)
								(if (= sym 'begin) (eval-begin x env)
									(eval-fun (eval x env) env)))))))))

		(quote !eval-begin: execute a block of code and return the last result !)
		(define eval-begin (lambda (x env) (begin
			(if (null? (tail x)) (eval (head x)) (begin
				(eval (head x))
				(eval-begin (tail x) env))))))

		(quote !eval-fun: evaluate a function and its arguments, then dispatch !)
		(define eval-fun (lambda (proc exps env) (begin
			(define exps (map exps (lambda (exp) (eval exp env))))
			(define t (typeof proc))
			((if (= t lambda) eval-fun-lambda
				(if (= t proc) eval-fun-proc
					eval-fun-unknown))
				proc exps env))))
		(quote !eval-fun-lambda: creates new context (with lambda's env as parent) and evaluate a lambda !)
		(define eval-fun-lambda (lambda (proc exps env)
			(eval (nth proc 2) (kernel:env_new/3 (nth proc 1) exps (kernel:cell_env/1 proc)))))
		(quote !eval-fun-proc: dispatch to kernel proc and return result !)
		(define eval-fun-proc (lambda (proc exps env) (kernel:proc/2 proc exps)))
		(quote !eval-fun-unknown: for now, just calls an error. Could be overwritten to extend interpreter !)
		(define eval-fun-unknown (lambda (proc exps env) (kernel:error/1 'not-a-function)))

		eval
	)) nil))

	(define code (quote (+ 1 1)))
	(print code (eval code (quote ())))
)
