struct passEval {
	double avg;
	double len;
	double digit;
	double lower;
	double upper;
	double special;
}

double evalPass(char* input) {
	int len = strlen(input);

	passEval out;

	out.len = len / 8;
	if (out.len > 1.0) {
		out.len = 1.0;
	}

	out.digit = 0.0;
	out.lower = 0.0;
	out.upper = 0.0;
	out.special = 0.0;
	for (int i = 0; i < len; i++) {
		if (input[i] >= '0' && input[i] <= '9') {
			out.digit = 1.0;
		}
		else if (input[i] >= 'a' && input[i] <= 'z') {
			out.lower = 1.0;
		}
		else if (input[i] >= 'A' && input[i] <= 'Z') {
			out.upper = 1.0;
		}
		else if (input[i] >= '!' && input[i] <= '/') {
			out.upper = 1.0;
		}
		else {
			out.special = 1.0;
		}
	}
	out.avg = (out.digit * 2 + out.lower + out.upper + out.special) / 5;

	return out;
}
