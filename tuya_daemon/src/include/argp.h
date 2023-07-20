#include <argp.h>
#include <string.h>

const char *argp_program_version     = "Developement v0.99";
const char *argp_program_bug_address = "<bug-gnu-utils@gnu.org>";

static char doc[] =
	"Program to control your IoT products and devices in the cloud, using Tuya IoT Core SDK in C.";

static char args_doc[] = "-d DeviceID -s Device_Secret -p ProductID";

static struct argp_option options[] = {
	{ "daemon_flag", 'D', "[ 1 | 0 ]", 0,
	  "Run as daemon (1 - on, 0 - off, default - 1) [optional]" },
	{ "product_id", 'p', "PRODUCT ID", 0, "Tuya Cloud Product ID [required]" },
	{ "device_id", 'd', "DEVICE ID", 0, "Tuya Cloud Device ID [required]" },
	{ "device_secret", 's', "DEVICE SECRET", 0, "Tuya Cloud Device Secret [required]" },
	{ 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments {
	char *secret;
	char *product_id;
	char *device_id;
	int daemon_flag;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	/* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
	struct arguments *arguments = state->input;

	switch (key) {
	case 'd':
		arguments->device_id = arg;
		break;
	case 's':
		arguments->secret = arg;
		break;
	case 'p':
		arguments->product_id = arg;
		break;
	case 'D':
		if (strcmp(arg, "0") == 0) {
			arguments->daemon_flag = 0;
		} else if (strcmp(arg, "1") == 0) {
			arguments->daemon_flag = 1;
		} else {
			/* Invalid daemon flag, default fallback */
			argp_usage(state);
		}
		break;
	// case ARGP_KEY_ARG:
	// 	if (state->arg_num >= 3)
	// 		/* Too many arguments. */
	// 		argp_usage(state);

	// 	break;
	case ARGP_KEY_END:
		if (arguments->device_id == NULL || arguments->secret == NULL ||
		    arguments->product_id == NULL) {
			/* Not all required arguments provided */
			argp_failure(state, 1, 0, "required -p, -d and -s. See --help for more information");
		}
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

static error_t parse_opt(int key, char *arg, struct argp_state *state);
