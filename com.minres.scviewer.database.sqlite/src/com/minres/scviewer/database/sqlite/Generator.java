package com.minres.scviewer.database.sqlite;

import java.util.List;

import com.minres.scviewer.database.ITrGenerator;
import com.minres.scviewer.database.ITrStream;
import com.minres.scviewer.database.ITransaction;
import com.minres.scviewer.database.sqlite.tables.ScvGenerator;

public class Generator implements ITrGenerator {

	private ITrStream  stream;
	private ScvGenerator scvGenerator;
	public Generator(ITrStream  stream, ScvGenerator scvGenerator) {
		this.stream=stream;
		this.scvGenerator=scvGenerator;
	}

	@Override
	public Long getId() {
		return (long) scvGenerator.getId();
	}

	@Override
	public ITrStream getStream() {
		return stream;
	}

	@Override
	public String getName() {
		return scvGenerator.getName();
	}

	@Override
	public List<ITransaction> getTransactions() {
		return null;
	}

}
