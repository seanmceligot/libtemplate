#ifndef __DEBUG_H
#define __DEBUG_H

#define debugf(fmt, args...) if(g_verbose_debug){fprintf(stderr, "debug: %s:%d " fmt "\n" , __FILE__, __LINE__, ## args);}
#define debug(msg) if(g_verbose_debug){fprintf(stderr, "debug: %s:%d %s\n", __FILE__, __LINE__, msg);}
#define errorf(fmt, args...) if(g_verbose_error){fprintf(stderr, "error: %s:%d " fmt "\n" , __FILE__, __LINE__, ## args);}
#define error(msg) if(g_verbose_error){fprintf(stderr, "error: %s:%d %s\n", __FILE__, __LINE__, msg);}
#define fatalf(fmt, args...) if(g_verbose_fatal){fprintf(stderr, "fatal: %s:%d " fmt "\n" , __FILE__, __LINE__, ## args);}
#define fatal(msg) if(g_verbose_fatal){fprintf(stderr, "fatal: %s:%d %s\n", __FILE__, __LINE__, msg);}
#define verbosef(fmt, args...) if(g_verbose_verbose){fprintf(stderr, "verbose: %s:%d " fmt "\n" , __FILE__, __LINE__, ## args);}
#define verbose(msg) if(g_verbose_verbose){fprintf(stderr, "verbose: %s:%d %s\n", __FILE__, __LINE__, msg);}
#define failf(fmt, args...) if (g_verbose_error) {fprintf(stderr, "fail: %s:%d " fmt "\n" , __FILE__, __LINE__, ## args);g_fail++;}
#define fail(msg) if(g_verbose_fail){fprintf(stderr, "fail: %s:%d %s\n", __FILE__, __LINE__, msg);g_fail++;}

#define assertf(b, fmt, args...) if (!(b)) { failf(fmt, args); }
#define assert(b, msg) if (!(b)) { fail(msg);}
#define assert_str_equals(s1,s2) if (strcmp(s1,s2)) {failf("(%s)!=(%s)",s1,s2);}


#endif
