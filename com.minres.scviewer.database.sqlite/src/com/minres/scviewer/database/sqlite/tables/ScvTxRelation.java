package com.minres.scviewer.database.sqlite.tables;

public class ScvTxRelation {
	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public int getSrc() {
		return src;
	}

	public void setSrc(int src) {
		this.src = src;
	}

	public int getSink() {
		return sink;
	}

	public void setSink(int sink) {
		this.sink = sink;
	}

	private String name;
	private int src;
	private int sink;

}