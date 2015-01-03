package com.minres.scviewer.database.sqlite;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

import com.minres.scviewer.database.sqlite.db.IDatabase;

public class SQLiteDatabase implements IDatabase {

	protected String dbFileName;
	
	public SQLiteDatabase(String dbFileName) {
		super();
		this.dbFileName = dbFileName;
	}

	@Override
	public Connection createConnection() throws SQLException {
        // create a database connection and return it
		return DriverManager.getConnection(getConnectionUrl() );
	}

	@Override
	public String getConnectionUrl() {
        // now we set up a set of fairly basic string variables to use in the body of the code proper
        String sJdbc = "jdbc:sqlite";
        String sDbUrl = sJdbc + ":" + dbFileName;
        // which will produce a legitimate Url for SqlLite JDBC :
        // jdbc:sqlite:hello.db
		return sDbUrl;
	}

	@Override
	public void close(ResultSet resultSet, Statement statement, Connection connection) {
		// TODO Auto-generated method stub

	}

	@Override
	public void close(PreparedStatement preparedStatement, Connection connection) {
		// TODO Auto-generated method stub

	}

}
