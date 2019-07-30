create table user_info_t(
		id			bigint		auto_increment,
		c_mail 		char(64)	not null	comment "user e_mail",
		u_name 		char(32) 	not null	comment "user name",
		u_sex 		char(32) 	null		comment "user sex",
		u_age		int		 	null		comment "user age",
		u_photo		char(32)	null		comment "head of user appear",
		u_area		char(16)	null		comment "user location",
		u_tag		char(255)	null		comment "interest of user",
		u_member	int			default 0	comment "news reader member",
		primary key(`id`),
		foreign key(c_mail) references check_user_t(c_mail) on delete cascade on update cascade,
		index(`c_mail`)

)ENGINE=InnoDB DEFAULT CHARSET='utf8';

