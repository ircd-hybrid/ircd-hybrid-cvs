#ifndef YYERRCODE
#define YYERRCODE 256
#endif

#define ACCEPT 257
#define ACCEPT_PASSWORD 258
#define ACTION 259
#define ADMIN 260
#define AUTH 261
#define AUTOCONN 262
#define CLASS 263
#define CONNECT 264
#define CONNECTFREQ 265
#define DENY 266
#define DESCRIPTION 267
#define DIE 268
#define DOTS_IN_IDENT 269
#define EMAIL 270
#define EXCEED_LIMIT 271
#define FNAME_USERLOG 272
#define FNAME_OPERLOG 273
#define FNAME_FOPERLOG 274
#define GECOS 275
#define GLINE 276
#define GLINES 277
#define GLINE_EXEMPT 278
#define GLINE_TIME 279
#define GLINE_LOG 280
#define GLOBAL_KILL 281
#define HAVE_IDENT 282
#define HOST 283
#define HUB 284
#define HUB_MASK 285
#define IDLETIME 286
#define INCLUDE 287
#define IP 288
#define IP_TYPE 289
#define KILL 290
#define KLINE 291
#define KLINE_EXEMPT 292
#define LAZYLINK 293
#define LEAF_MASK 294
#define LISTEN 295
#define LOGGING 296
#define T_LOGPATH 297
#define LOG_LEVEL 298
#define MAX_NUMBER 299
#define MAXIMUM_LINKS 300
#define MESSAGE_LOCALE 301
#define NAME 302
#define NETWORK_NAME 303
#define NETWORK_DESC 304
#define NICK_CHANGES 305
#define NO_TILDE 306
#define NUMBER 307
#define NUMBER_PER_IP 308
#define OPERATOR 309
#define OPER_LOG 310
#define PASSWORD 311
#define PING_TIME 312
#define PORT 313
#define QSTRING 314
#define QUARANTINE 315
#define QUIET_ON_BAN 316
#define REASON 317
#define REDIRSERV 318
#define REDIRPORT 319
#define REHASH 320
#define REMOTE 321
#define SENDQ 322
#define SEND_PASSWORD 323
#define SERVERINFO 324
#define SERVER_MASK 325
#define SHARED 326
#define SPOOF 327
#define TREJECT 328
#define TNO 329
#define TYES 330
#define T_L_CRIT 331
#define T_L_DEBUG 332
#define T_L_ERROR 333
#define T_L_INFO 334
#define T_L_NOTICE 335
#define T_L_TRACE 336
#define T_L_WARN 337
#define UNKLINE 338
#define USER 339
#define VHOST 340
#define WARN 341
#define SILENT 342
#define GENERAL 343
#define FAILED_OPER_NOTICE 344
#define SHOW_FAILED_OPER_ID 345
#define ANTI_NICK_FLOOD 346
#define MAX_NICK_TIME 347
#define MAX_NICK_CHANGES 348
#define TS_MAX_DELTA 349
#define TS_WARN_DELTA 350
#define KLINE_WITH_REASON 351
#define KLINE_WITH_CONNECTION_CLOSED 352
#define WARN_NO_NLINE 353
#define NON_REDUNDANT_KLINES 354
#define E_LINES_OPER_ONLY 355
#define F_LINES_OPER_ONLY 356
#define O_LINES_OPER_ONLY 357
#define STATS_NOTICE 358
#define WHOIS_WAIT 359
#define PACE_WAIT 360
#define KNOCK_DELAY 361
#define SHORT_MOTD 362
#define NO_OPER_FLOOD 363
#define IAUTH_SERVER 364
#define IAUTH_PORT 365
#define STATS_P_NOTICE 366
#define INVITE_PLUS_I_ONLY 367
#define MODULE 368
#define MODULES 369
#define HIDESERVER 370
#define CLIENT_EXIT 371
#define T_BOTS 372
#define T_CCONN 373
#define T_DEBUG 374
#define T_DRONE 375
#define T_FULL 376
#define T_SKILL 377
#define T_LOCOPS 378
#define T_NCHANGE 379
#define T_REJ 380
#define T_UNAUTH 381
#define T_SPY 382
#define T_EXTERNAL 383
#define T_OPERWALL 384
#define T_SERVNOTICE 385
#define T_INVISIBLE 386
#define T_CALLERID 387
#define T_WALLOP 388
#define OPER_ONLY_UMODES 389
#define PATH 390
#define MAX_TARGETS 391
#define T_MAX_CLIENTS 392
#define LINKS_NOTICE 393
#define LINKS_DELAY 394
#define VCHANS_OPER_ONLY 395
typedef union {
        int  number;
        char *string;
        struct ip_value ip_entry;
} YYSTYPE;
extern YYSTYPE yylval;
