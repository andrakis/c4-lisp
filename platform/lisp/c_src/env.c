#include "./lisp.h"

void env__drop_reference (Env *env) {
	if(!(--env->ref_count)) {
		env__free(env);
	}
}

