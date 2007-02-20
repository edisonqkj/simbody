#include "chemistry/AminoAcid.h"
#include <iostream> // NULL

class AminoAcidRep {
	friend class AminoAcid;
private:
	explicit AminoAcidRep(const AminoAcidType & aaType, const AminoAcid * handle) 
		: myHandle(handle)
	{
		initialize(&aaType);
	}

	explicit AminoAcidRep(char oneLetterCode, const AminoAcid * handle) 
		: myHandle(handle)
	{
		const AminoAcidType * t = AminoAcidType::typeFor(oneLetterCode);
		initialize(t);
	}

	void initialize(const AminoAcidType * aaType) {
		type = aaType;
	}

	char oneLetterCode() const {
		if (type == NULL) return 'X';
		return type->oneLetterCode();
	}
	
	AminoAcidRep* clone() const {
		AminoAcidRep* dup = new AminoAcidRep(*this);
		return dup;
	}

	bool operator!=(const AminoAcidRep & other) const {
		if (*type != *(other.type)) return true;
		return false;
	}
	bool operator==(const AminoAcidRep & other) const {
		return !(*this != other);
	}

	int number() const {
		return residueNumber;
	}

	const AminoAcidType * type;
	const AminoAcid * myHandle;
	int residueNumber;
};

AminoAcid::AminoAcid(const AminoAcidType & type, int resNum)
{
	rep = new AminoAcidRep(type, this);
	rep->residueNumber = resNum;
}

AminoAcid::AminoAcid(char oneLetterCode, int resNum)
{
	rep = new AminoAcidRep(oneLetterCode, this);
	rep->residueNumber = resNum;
}

AminoAcid::~AminoAcid()
{
	if (rep && (this == rep->myHandle)) delete rep;
	rep = NULL;
}
// Copy constructor
AminoAcid::AminoAcid(const AminoAcid & src) {
	if (this == &src) return;
	rep = src.rep->clone();
	rep->myHandle = this;
}
AminoAcid & AminoAcid::operator=(const AminoAcid & src) {
	if (rep && (this == rep->myHandle)) delete rep;
	rep = src.rep->clone();
	rep->myHandle = this;
	return *this;
}
bool AminoAcid::operator==(const AminoAcid & src) const {return (*(this->rep)) == (*(src.rep));}
bool AminoAcid::operator!=(const AminoAcid & src) const {return (*(this->rep)) != (*(src.rep));}

char AminoAcid::oneLetterCode() const {return rep->oneLetterCode();}

int AminoAcid::number() const{
	return rep->number();
}
