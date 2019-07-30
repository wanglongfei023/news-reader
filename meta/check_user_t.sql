create table check_user_t(
		c_mail 		char(64)	not null	comment "user e_mail",
		c_name 		char(32) 	not null	comment "user name",
		c_passwd 	char(32) 	not null	comment "user passwd",
		c_state		int		 	not null	comment "user state",
		primary key(`c_mail`),
		index(`c_mail`)

)ENGINE=InnoDB DEFAULT CHARSET='utf8';
