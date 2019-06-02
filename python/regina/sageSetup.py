# Additional initialisation that takes place only when running Regina
# from within Sage:

# -------------------------------------------------------------------------
# Sage-related hacks implemented at the C++ level in Regina
# -------------------------------------------------------------------------

# These hacks include making boost::python work with Sage's Integer type.
engine._addSageHacks()

# -------------------------------------------------------------------------
# Census data location
# -------------------------------------------------------------------------

# In sageRegina, the census files are supplied differently.
# Set the location here.
try:
    from .pyCensus import __path__ as _pyCensusPath
    GlobalDirs.setDirs(
        GlobalDirs.home(),
        GlobalDirs.pythonModule(),
        _pyCensusPath[0])
except ImportError:
    pass

# -------------------------------------------------------------------------
# Conversion from Regina objects to Sage objects
# -------------------------------------------------------------------------

from .engine import GroupPresentation

# Additional methods for GroupPresentation

# Follow convention that .sage() yields a representation of
# a mathematical object native to sage

def _convertRel(generators, rel):
    return sage.all.prod([
            generators[term.generator] ** term.exponent
            for term in rel.terms()])

def _convertGroupPresentation(self):
    """
    Returns a Sage version of this finitely presented group.
    """
    F = sage.all.FreeGroup(self.countGenerators())
    gens = F.generators()
    rels = [
        _convertRel(gens, self.relation(i))
        for i in range(self.countRelations())]
    return F/rels

GroupPresentation.sage = _convertGroupPresentation
