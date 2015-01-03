package com.minres.scviewer.database.sqlite.tables;

public class ScvTxEvent {
	public int getTx() {
		return tx;
	}

	public void setTx(int tx) {
		this.tx = tx;
	}

	public int getType() {
		return type;
	}

	public void setType(int type) {
		this.type = type;
	}

	public long getTime() {
		return time;
	}

	public void setTime(long time) {
		this.time = time;
	}

	private int tx;
	private int type;
	private long time;
}