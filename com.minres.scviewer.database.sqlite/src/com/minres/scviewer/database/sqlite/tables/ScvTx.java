package com.minres.scviewer.database.sqlite.tables;

public class ScvTx {
	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getGenerator() {
		return generator;
	}

	public void setGenerator(int generator) {
		this.generator = generator;
	}

	public int getStream() {
		return stream;
	}

	public void setStream(int stream) {
		this.stream = stream;
	}

	public int getConcurrencyLevel() {
		return concurrencyLevel;
	}

	public void setConcurrencyLevel(int concurrencyLevel) {
		this.concurrencyLevel = concurrencyLevel;
	}

	private int id;
	private int generator;
	private int stream;
	private int concurrencyLevel;
}