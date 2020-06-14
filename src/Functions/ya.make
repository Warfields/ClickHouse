# This file is generated automatically, do not edit. See 'ya.make.in' and use 'utils/generate-ya-make' to regenerate it.
OWNER(g:clickhouse)

LIBRARY()

CFLAGS(
    -DUSE_H3 -DUSE_SSL -DUSE_XXHASH
)

ADDINCL(
    library/cpp/consistent_hashing
    contrib/libs/farmhash
    contrib/libs/h3/h3lib/include
    contrib/libs/hyperscan/src
    contrib/libs/libdivide
    contrib/libs/rapidjson/include
    contrib/libs/xxhash
)

PEERDIR(
    clickhouse/src/Common
    clickhouse/src/Parsers
    clickhouse/src/Dictionaries
    contrib/libs/farmhash
    contrib/libs/fastops/fastops
    contrib/libs/h3
    contrib/libs/hyperscan
    contrib/libs/icu
    contrib/libs/libdivide
    contrib/libs/metrohash
    contrib/libs/rapidjson
    contrib/libs/xxhash
    library/cpp/consistent_hashing
)

# "Arcadia" build is slightly deficient. It lacks many libraries that we need.

SRCS(
    CRC.cpp
    FunctionFQDN.cpp
    FunctionFactory.cpp
    FunctionFile.cpp
    FunctionHelpers.cpp
    FunctionJoinGet.cpp
    FunctionsAES.cpp
    FunctionsCoding.cpp
    FunctionsConversion.cpp
    FunctionsEmbeddedDictionaries.cpp
    FunctionsExternalDictionaries.cpp
    FunctionsExternalModels.cpp
    FunctionsHashing.cpp
    FunctionsJSON.cpp
    FunctionsLogical.cpp
    FunctionsRandom.cpp
    FunctionsRound.cpp
    FunctionsStringArray.cpp
    FunctionsStringHash.cpp
    FunctionsStringSimilarity.cpp
    GatherUtils/concat.cpp
    GatherUtils/createArraySink.cpp
    GatherUtils/createArraySource.cpp
    GatherUtils/createValueSource.cpp
    GatherUtils/has_all.cpp
    GatherUtils/has_any.cpp
    GatherUtils/has_substr.cpp
    GatherUtils/push.cpp
    GatherUtils/resizeConstantSize.cpp
    GatherUtils/resizeDynamicSize.cpp
    GatherUtils/sliceDynamicOffsetBounded.cpp
    GatherUtils/sliceDynamicOffsetUnbounded.cpp
    GatherUtils/sliceFromLeftConstantOffsetBounded.cpp
    GatherUtils/sliceFromLeftConstantOffsetUnbounded.cpp
    GatherUtils/sliceFromRightConstantOffsetBounded.cpp
    GatherUtils/sliceFromRightConstantOffsetUnbounded.cpp
    GeoHash.cpp
    IFunction.cpp
    TargetSpecific.cpp
    URL/URLHierarchy.cpp
    URL/URLPathHierarchy.cpp
    URL/basename.cpp
    URL/cutFragment.cpp
    URL/cutQueryString.cpp
    URL/cutQueryStringAndFragment.cpp
    URL/cutToFirstSignificantSubdomain.cpp
    URL/cutToFirstSignificantSubdomainCustom.cpp
    URL/cutURLParameter.cpp
    URL/cutWWW.cpp
    URL/decodeURLComponent.cpp
    URL/domain.cpp
    URL/domainWithoutWWW.cpp
    URL/extractURLParameter.cpp
    URL/extractURLParameterNames.cpp
    URL/extractURLParameters.cpp
    URL/firstSignificantSubdomain.cpp
    URL/firstSignificantSubdomainCustom.cpp
    URL/fragment.cpp
    URL/netloc.cpp
    URL/path.cpp
    URL/pathFull.cpp
    URL/port.cpp
    URL/protocol.cpp
    URL/queryString.cpp
    URL/queryStringAndFragment.cpp
    URL/registerFunctionsURL.cpp
    URL/tldLookup.generated.cpp
    URL/topLevelDomain.cpp
    abs.cpp
    acos.cpp
    acosh.cpp
    addDays.cpp
    addHours.cpp
    addMinutes.cpp
    addMonths.cpp
    addQuarters.cpp
    addSeconds.cpp
    addWeeks.cpp
    addYears.cpp
    addressToLine.cpp
    addressToSymbol.cpp
    aes_decrypt_mysql.cpp
    aes_encrypt_mysql.cpp
    appendTrailingCharIfAbsent.cpp
    array/array.cpp
    array/arrayAUC.cpp
    array/arrayAggregation.cpp
    array/arrayAll.cpp
    array/arrayCompact.cpp
    array/arrayConcat.cpp
    array/arrayCount.cpp
    array/arrayCumSum.cpp
    array/arrayCumSumNonNegative.cpp
    array/arrayDifference.cpp
    array/arrayDistinct.cpp
    array/arrayElement.cpp
    array/arrayEnumerate.cpp
    array/arrayEnumerateDense.cpp
    array/arrayEnumerateDenseRanked.cpp
    array/arrayEnumerateRanked.cpp
    array/arrayEnumerateUniq.cpp
    array/arrayEnumerateUniqRanked.cpp
    array/arrayExists.cpp
    array/arrayFill.cpp
    array/arrayFilter.cpp
    array/arrayFirst.cpp
    array/arrayFirstIndex.cpp
    array/arrayFlatten.cpp
    array/arrayIntersect.cpp
    array/arrayJoin.cpp
    array/arrayMap.cpp
    array/arrayPopBack.cpp
    array/arrayPopFront.cpp
    array/arrayPushBack.cpp
    array/arrayPushFront.cpp
    array/arrayReduce.cpp
    array/arrayReduceInRanges.cpp
    array/arrayResize.cpp
    array/arrayReverse.cpp
    array/arraySlice.cpp
    array/arraySort.cpp
    array/arraySplit.cpp
    array/arrayUniq.cpp
    array/arrayWithConstant.cpp
    array/arrayZip.cpp
    array/countEqual.cpp
    array/emptyArray.cpp
    array/emptyArrayToSingle.cpp
    array/has.cpp
    array/hasAll.cpp
    array/hasAny.cpp
    array/hasSubstr.cpp
    array/indexOf.cpp
    array/length.cpp
    array/mapOp.cpp
    array/mapPopulateSeries.cpp
    array/range.cpp
    array/registerFunctionsArray.cpp
    asin.cpp
    asinh.cpp
    assumeNotNull.cpp
    atan.cpp
    atan2.cpp
    atanh.cpp
    bar.cpp
    base64Decode.cpp
    base64Encode.cpp
    bitAnd.cpp
    bitBoolMaskAnd.cpp
    bitBoolMaskOr.cpp
    bitCount.cpp
    bitHammingDistance.cpp
    bitNot.cpp
    bitOr.cpp
    bitRotateLeft.cpp
    bitRotateRight.cpp
    bitShiftLeft.cpp
    bitShiftRight.cpp
    bitSwapLastTwo.cpp
    bitTest.cpp
    bitTestAll.cpp
    bitTestAny.cpp
    bitWrapperFunc.cpp
    bitXor.cpp
    bitmaskToList.cpp
    blockNumber.cpp
    blockSerializedSize.cpp
    blockSize.cpp
    buildId.cpp
    byteSize.cpp
    caseWithExpression.cpp
    cbrt.cpp
    coalesce.cpp
    concat.cpp
    connectionID.cpp
    convertCharset.cpp
    cos.cpp
    cosh.cpp
    countDigits.cpp
    countMatches.cpp
    countSubstrings.cpp
    countSubstringsCaseInsensitive.cpp
    countSubstringsCaseInsensitiveUTF8.cpp
    currentDatabase.cpp
    currentUser.cpp
    dateDiff.cpp
    date_trunc.cpp
    decodeXMLComponent.cpp
    decrypt.cpp
    defaultValueOfArgumentType.cpp
    defaultValueOfTypeName.cpp
    demange.cpp
    divide.cpp
    dumpColumnStructure.cpp
    e.cpp
    empty.cpp
    encodeXMLComponent.cpp
    encrypt.cpp
    endsWith.cpp
    equals.cpp
    erf.cpp
    erfc.cpp
    errorCodeToName.cpp
    evalMLMethod.cpp
    exp.cpp
    exp10.cpp
    exp2.cpp
    extract.cpp
    extractAllGroupsHorizontal.cpp
    extractAllGroupsVertical.cpp
    extractGroups.cpp
    extractTextFromHTML.cpp
    extractTimeZoneFromFunctionArguments.cpp
    filesystem.cpp
    finalizeAggregation.cpp
    formatDateTime.cpp
    formatReadableQuantity.cpp
    formatReadableSize.cpp
    formatReadableTimeDelta.cpp
    formatRow.cpp
    formatString.cpp
    fromModifiedJulianDay.cpp
    fromUnixTimestamp64Micro.cpp
    fromUnixTimestamp64Milli.cpp
    fromUnixTimestamp64Nano.cpp
    fuzzBits.cpp
    gcd.cpp
    generateUUIDv4.cpp
    geoToH3.cpp
    geohashDecode.cpp
    geohashEncode.cpp
    geohashesInBox.cpp
    geometryConverters.cpp
    getMacro.cpp
    getScalar.cpp
    getSetting.cpp
    getSizeOfEnumType.cpp
    globalVariable.cpp
    greatCircleDistance.cpp
    greater.cpp
    greaterOrEquals.cpp
    greatest.cpp
    h3EdgeAngle.cpp
    h3EdgeLengthM.cpp
    h3GetBaseCell.cpp
    h3GetResolution.cpp
    h3HexAreaM2.cpp
    h3IndexesAreNeighbors.cpp
    h3IsValid.cpp
    h3ToChildren.cpp
    h3ToParent.cpp
    h3ToString.cpp
    h3kRing.cpp
    hasColumnInTable.cpp
    hasThreadFuzzer.cpp
    hasToken.cpp
    hasTokenCaseInsensitive.cpp
    hostName.cpp
    hypot.cpp
    identity.cpp
    if.cpp
    ifNotFinite.cpp
    ifNull.cpp
    ignore.cpp
    ilike.cpp
    in.cpp
    initializeAggregation.cpp
    intDiv.cpp
    intDivOrZero.cpp
    intExp10.cpp
    intExp2.cpp
    isConstant.cpp
    isDecimalOverflow.cpp
    isFinite.cpp
    isInfinite.cpp
    isNaN.cpp
    isNotNull.cpp
    isNull.cpp
    isValidUTF8.cpp
    isZeroOrNull.cpp
    jumpConsistentHash.cpp
    lcm.cpp
    least.cpp
    lengthUTF8.cpp
    less.cpp
    lessOrEquals.cpp
    lgamma.cpp
    like.cpp
    log.cpp
    log10.cpp
    log1p.cpp
    log2.cpp
    logTrace.cpp
    lowCardinalityIndices.cpp
    lowCardinalityKeys.cpp
    lower.cpp
    lowerUTF8.cpp
    map.cpp
    match.cpp
    materialize.cpp
    minus.cpp
    modulo.cpp
    moduloOrZero.cpp
    multiFuzzyMatchAllIndices.cpp
    multiFuzzyMatchAny.cpp
    multiFuzzyMatchAnyIndex.cpp
    multiIf.cpp
    multiMatchAllIndices.cpp
    multiMatchAny.cpp
    multiMatchAnyIndex.cpp
    multiSearchAllPositions.cpp
    multiSearchAllPositionsCaseInsensitive.cpp
    multiSearchAllPositionsCaseInsensitiveUTF8.cpp
    multiSearchAllPositionsUTF8.cpp
    multiSearchAny.cpp
    multiSearchAnyCaseInsensitive.cpp
    multiSearchAnyCaseInsensitiveUTF8.cpp
    multiSearchAnyUTF8.cpp
    multiSearchFirstIndex.cpp
    multiSearchFirstIndexCaseInsensitive.cpp
    multiSearchFirstIndexCaseInsensitiveUTF8.cpp
    multiSearchFirstIndexUTF8.cpp
    multiSearchFirstPosition.cpp
    multiSearchFirstPositionCaseInsensitive.cpp
    multiSearchFirstPositionCaseInsensitiveUTF8.cpp
    multiSearchFirstPositionUTF8.cpp
    multiply.cpp
    negate.cpp
    neighbor.cpp
    normalizeQuery.cpp
    normalizedQueryHash.cpp
    notEmpty.cpp
    notEquals.cpp
    notILike.cpp
    notLike.cpp
    now.cpp
    now64.cpp
    nullIf.cpp
    pi.cpp
    plus.cpp
    pointInEllipses.cpp
    pointInPolygon.cpp
    position.cpp
    positionCaseInsensitive.cpp
    positionCaseInsensitiveUTF8.cpp
    positionUTF8.cpp
    pow.cpp
    rand.cpp
    rand64.cpp
    randConstant.cpp
    randomFixedString.cpp
    randomPrintableASCII.cpp
    randomString.cpp
    randomStringUTF8.cpp
    regexpQuoteMeta.cpp
    registerFunctions.cpp
    registerFunctionsArithmetic.cpp
    registerFunctionsComparison.cpp
    registerFunctionsConditional.cpp
    registerFunctionsConsistentHashing.cpp
    registerFunctionsDateTime.cpp
    registerFunctionsFormatting.cpp
    registerFunctionsGeo.cpp
    registerFunctionsHigherOrder.cpp
    registerFunctionsIntrospection.cpp
    registerFunctionsMath.cpp
    registerFunctionsMiscellaneous.cpp
    registerFunctionsNull.cpp
    registerFunctionsRandom.cpp
    registerFunctionsReinterpret.cpp
    registerFunctionsString.cpp
    registerFunctionsStringRegexp.cpp
    registerFunctionsStringSearch.cpp
    registerFunctionsTuple.cpp
    registerFunctionsUnixTimestamp64.cpp
    registerFunctionsVisitParam.cpp
    reinterpretAs.cpp
    repeat.cpp
    replaceAll.cpp
    replaceOne.cpp
    replaceRegexpAll.cpp
    replaceRegexpOne.cpp
    replicate.cpp
    reverse.cpp
    reverseUTF8.cpp
    roundAge.cpp
    roundDuration.cpp
    roundToExp2.cpp
    rowNumberInAllBlocks.cpp
    rowNumberInBlock.cpp
    runningAccumulate.cpp
    runningConcurrency.cpp
    runningDifference.cpp
    runningDifferenceStartingWithFirstValue.cpp
    sigmoid.cpp
    sign.cpp
    sin.cpp
    sinh.cpp
    sleep.cpp
    sleepEachRow.cpp
    sqrt.cpp
    startsWith.cpp
    stringToH3.cpp
    substring.cpp
    subtractDays.cpp
    subtractHours.cpp
    subtractMinutes.cpp
    subtractMonths.cpp
    subtractQuarters.cpp
    subtractSeconds.cpp
    subtractWeeks.cpp
    subtractYears.cpp
    tan.cpp
    tanh.cpp
    tcpPort.cpp
    tgamma.cpp
    throwIf.cpp
    tid.cpp
    timeSlot.cpp
    timeSlots.cpp
    timezone.cpp
    timezoneOffset.cpp
    toColumnTypeName.cpp
    toCustomWeek.cpp
    toDayOfMonth.cpp
    toDayOfWeek.cpp
    toDayOfYear.cpp
    toFixedString.cpp
    toHour.cpp
    toISOWeek.cpp
    toISOYear.cpp
    toLowCardinality.cpp
    toMinute.cpp
    toModifiedJulianDay.cpp
    toMonday.cpp
    toMonth.cpp
    toNullable.cpp
    toQuarter.cpp
    toRelativeDayNum.cpp
    toRelativeHourNum.cpp
    toRelativeMinuteNum.cpp
    toRelativeMonthNum.cpp
    toRelativeQuarterNum.cpp
    toRelativeSecondNum.cpp
    toRelativeWeekNum.cpp
    toRelativeYearNum.cpp
    toSecond.cpp
    toStartOfDay.cpp
    toStartOfFifteenMinutes.cpp
    toStartOfFiveMinute.cpp
    toStartOfHour.cpp
    toStartOfISOYear.cpp
    toStartOfInterval.cpp
    toStartOfMinute.cpp
    toStartOfMonth.cpp
    toStartOfQuarter.cpp
    toStartOfSecond.cpp
    toStartOfTenMinutes.cpp
    toStartOfYear.cpp
    toTime.cpp
    toTimeZone.cpp
    toTypeName.cpp
    toUnixTimestamp64Micro.cpp
    toUnixTimestamp64Milli.cpp
    toUnixTimestamp64Nano.cpp
    toValidUTF8.cpp
    toYYYYMM.cpp
    toYYYYMMDD.cpp
    toYYYYMMDDhhmmss.cpp
    toYear.cpp
    today.cpp
    transform.cpp
    trap.cpp
    trim.cpp
    tryBase64Decode.cpp
    tuple.cpp
    tupleElement.cpp
    tupleHammingDistance.cpp
    upper.cpp
    upperUTF8.cpp
    uptime.cpp
    version.cpp
    visibleWidth.cpp
    visitParamExtractBool.cpp
    visitParamExtractFloat.cpp
    visitParamExtractInt.cpp
    visitParamExtractRaw.cpp
    visitParamExtractString.cpp
    visitParamExtractUInt.cpp
    visitParamHas.cpp
    wkt.cpp
    yandexConsistentHash.cpp
    yesterday.cpp

)

END()
